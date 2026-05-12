#!/usr/bin/env python3
"""
Webová aplikácia pre OPK Robot Duel.
Spustenie: python3 web_server.py
Otvorte: http://localhost:5000

Ovládanie:
  P1 (šípky) — klávesnica alebo tlačidlá na stránke
  P2 (WASD)  — klávesnica alebo tlačidlá na stránke
"""
import threading
import time
import os
import sys

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, Pose2D
from robot_msgs.msg import GameState
from visualization_msgs.msg import MarkerArray
from std_srvs.srv import Trigger

from flask import Flask, send_file, Response
from flask_socketio import SocketIO

# ─── Zdieľaný stav ────────────────────────────────────────────────────────────
state = {
    "p1": {"x": -7.0, "y": 6.0, "theta": 0.0},
    "p2": {"x": -5.0, "y": 6.0, "theta": 3.14},
    "game": {
        "time_left": 60.0, "game_over": False, "winner": "",
        "p1_score": 0, "p1_cap_used": 0, "p1_cap_max": 5,
        "p1_paper": 0, "p1_plastic": 0, "p1_glass": 0,
        "p2_score": 0, "p2_cap_used": 0, "p2_cap_max": 5,
        "p2_paper": 0, "p2_plastic": 0, "p2_glass": 0,
        "wastes_remaining": 0,
    },
    "wastes": [],   # [{x, y, r, type}]
}
state_lock = threading.Lock()

# ─── Flask + SocketIO ─────────────────────────────────────────────────────────
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")

MAP_PNG = os.path.join(os.path.dirname(__file__),
                       "../../../robot/cpp/config/resources/opk-map.png")

@app.route("/map.png")
def serve_map():
    return send_file(os.path.abspath(MAP_PNG), mimetype="image/png")

@app.route("/")
def index():
    return Response(HTML, mimetype="text/html")

@socketio.on("cmd_vel")
def handle_cmd_vel(data):
    # data = {"player": "p1"|"p2", "linear": float, "angular": float}
    msg = Twist()
    msg.linear.x  = float(data.get("linear",  0.0))
    msg.angular.z = float(data.get("angular", 0.0))
    player = data.get("player", "p1")
    if ros_node and rclpy.ok():
        if player == "p1":
            ros_node.p1_pub.publish(msg)
        else:
            ros_node.p2_pub.publish(msg)

@socketio.on("reset")
def handle_reset(_data=None):
    if ros_node and rclpy.ok():
        ros_node.reset_game()

# ─── ROS uzel ─────────────────────────────────────────────────────────────────
class WebBridgeNode(Node):
    LIN = 0.8
    ANG = 1.2

    def __init__(self):
        super().__init__("web_bridge")
        self.p1_pub = self.create_publisher(Twist, "/p1/robot/cmd_vel", 10)
        self.p2_pub = self.create_publisher(Twist, "/p2/robot/cmd_vel", 10)

        self.create_subscription(Pose2D,      "/p1/robot/pose",   self._p1_pose, 10)
        self.create_subscription(Pose2D,      "/p2/robot/pose",   self._p2_pose, 10)
        self.create_subscription(GameState,   "/game/state",      self._game_state, 10)
        self.create_subscription(MarkerArray, "/game/waste_markers", self._wastes, 10)

        self._reset_cli = self.create_client(Trigger, "/game/reset")

        # Push do klientov 20× za sekundu
        self.create_timer(0.05, self._push_state)

    # ── Callbacks ─────────────────────────────────────────────────────────────
    def _p1_pose(self, msg):
        with state_lock:
            state["p1"].update({"x": msg.x, "y": msg.y, "theta": msg.theta})

    def _p2_pose(self, msg):
        with state_lock:
            state["p2"].update({"x": msg.x, "y": msg.y, "theta": msg.theta})

    def _game_state(self, msg):
        with state_lock:
            g = state["game"]
            g["time_left"]      = msg.time_left
            g["game_over"]      = msg.game_over
            g["winner"]         = msg.winner
            g["p1_score"]       = msg.p1_score
            g["p1_cap_used"]    = msg.p1_capacity_used
            g["p1_cap_max"]     = msg.p1_capacity_max
            g["p1_paper"]       = msg.p1_paper
            g["p1_plastic"]     = msg.p1_plastic
            g["p1_glass"]       = msg.p1_glass
            g["p2_score"]       = msg.p2_score
            g["p2_cap_used"]    = msg.p2_capacity_used
            g["p2_cap_max"]     = msg.p2_capacity_max
            g["p2_paper"]       = msg.p2_paper
            g["p2_plastic"]     = msg.p2_plastic
            g["p2_glass"]       = msg.p2_glass
            g["wastes_remaining"] = msg.wastes_remaining

    def _wastes(self, msg):
        wastes = []
        for m in msg.markers:
            if m.action != 0:   # 0 = ADD
                continue
            if m.ns != "wastes":
                continue
            # radius z scale.x / 2 (diameter → radius)
            wastes.append({
                "x": m.pose.position.x,
                "y": m.pose.position.y,
                "r": m.scale.x / 2.0,
                "type": _marker_color_to_type(m.color.r, m.color.g, m.color.b),
            })
        with state_lock:
            state["wastes"] = wastes

    def _push_state(self):
        with state_lock:
            payload = {
                "p1":     dict(state["p1"]),
                "p2":     dict(state["p2"]),
                "game":   dict(state["game"]),
                "wastes": list(state["wastes"]),
            }
        socketio.emit("state", payload)

    def reset_game(self):
        if self._reset_cli.service_is_ready():
            self._reset_cli.call_async(Trigger.Request())


def _marker_color_to_type(r, g, b):
    """Vráti typ odpadu podľa farby markera game_node."""
    if r > 0.8 and g > 0.8 and b < 0.5:   # žltá / papier
        return "paper"
    if r > 0.8 and g < 0.5 and b < 0.5:   # červená / sklo (tmavé žlté)
        return "glass"
    return "plastic"


# ─── HTML / JS ────────────────────────────────────────────────────────────────
HTML = r"""<!DOCTYPE html>
<html lang="sk">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>OPK Robot Duel</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { background: #1a1a2e; color: #eee; font-family: monospace; display: flex;
         flex-direction: column; align-items: center; min-height: 100vh; padding: 8px; }
  h1  { font-size: 1.2rem; color: #ffd700; margin: 6px 0; letter-spacing: 2px; }
  #wrap { display: flex; gap: 10px; align-items: flex-start; flex-wrap: wrap; justify-content: center; }
  canvas { border: 2px solid #444; background: #000; image-rendering: pixelated;
           max-width: 100%; cursor: crosshair; }
  .panel { background: #16213e; border: 1px solid #444; border-radius: 6px;
           padding: 10px; min-width: 180px; }
  .panel h2 { font-size: .85rem; margin-bottom: 6px; }
  .p1c { color: #4cb8ff; } .p2c { color: #cc66ff; }
  .row { display: flex; justify-content: space-between; gap: 8px; font-size: .8rem; margin: 3px 0; }
  .score { font-size: 1.4rem; font-weight: bold; }
  .bar { width: 100%; height: 10px; background: #333; border-radius: 4px; margin: 4px 0; }
  .bar-fill { height: 100%; border-radius: 4px; transition: width .2s; }
  #timer { font-size: 1.6rem; text-align: center; margin: 6px 0; color: #ffd700; }
  #winner { font-size: 1rem; text-align: center; color: #ff6b6b; min-height: 20px; }
  #status { font-size: .7rem; color: #888; text-align: center; margin: 2px 0; }
  /* Gamepad */
  .gpad { display: grid; grid-template-columns: repeat(3,44px);
          grid-template-rows: repeat(3,44px); gap:3px; }
  .gbtn { background:#2a2a4a; border:1px solid #555; border-radius:6px;
          color:#ccc; font-size:1.2rem; cursor:pointer; user-select:none;
          display:flex; align-items:center; justify-content:center;
          transition: background .1s; }
  .gbtn:active, .gbtn.active { background:#4a4a8a; }
  .gpad-wrap { display: flex; flex-direction:column; align-items:center; gap:4px; }
  .gpad-label { font-size:.75rem; color:#aaa; }
  #controls { display: flex; gap: 16px; margin-top: 6px; flex-wrap:wrap; justify-content:center; }
  button.action { background:#1e3a5f; border:1px solid #4cb8ff; color:#4cb8ff;
                  padding:6px 14px; border-radius:4px; cursor:pointer; font-family:monospace; }
  button.action:hover { background:#2a5080; }
</style>
</head>
<body>
<h1>⚙ OPK ROBOT DUEL</h1>

<div id="wrap">
  <!-- Canvas -->
  <canvas id="cv" width="768" height="512"></canvas>

  <!-- Pravý panel -->
  <div style="display:flex;flex-direction:column;gap:8px">

    <!-- Timer -->
    <div class="panel">
      <div id="timer">60.0 s</div>
      <div id="winner"></div>
      <div id="status">čakám na ROS…</div>
    </div>

    <!-- P1 -->
    <div class="panel">
      <h2 class="p1c">▶ P1  (šípky / tlačidlá vľavo)</h2>
      <div class="row"><span>Skóre:</span><span class="score p1c" id="p1score">0</span></div>
      <div class="row"><span>Batoh:</span><span id="p1cap">0/5</span></div>
      <div class="bar"><div class="bar-fill" id="p1bar" style="background:#4cb8ff;width:0%"></div></div>
      <div class="row" style="font-size:.7rem">
        <span id="p1inv">prázdny</span>
      </div>
    </div>

    <!-- P2 -->
    <div class="panel">
      <h2 class="p2c">▶ P2  (WASD / tlačidlá vpravo)</h2>
      <div class="row"><span>Skóre:</span><span class="score p2c" id="p2score">0</span></div>
      <div class="row"><span>Batoh:</span><span id="p2cap">0/5</span></div>
      <div class="bar"><div class="bar-fill" id="p2bar" style="background:#cc66ff;width:0%"></div></div>
      <div class="row" style="font-size:.7rem">
        <span id="p2inv">prázdny</span>
      </div>
    </div>

    <!-- Akcie -->
    <div style="display:flex;gap:8px;justify-content:center">
      <button class="action" onclick="doReset()">↺ Reset</button>
    </div>

  </div>
</div>

<!-- Gamepady -->
<div id="controls">
  <div class="gpad-wrap">
    <div class="gpad-label p1c">P1 (šípky)</div>
    <div class="gpad">
      <div></div>
      <div class="gbtn" id="b-p1-up"    data-p="p1" data-lin="0.8"  data-ang="0">▲</div>
      <div></div>
      <div class="gbtn" id="b-p1-left"  data-p="p1" data-lin="0"    data-ang="1.2">◀</div>
      <div class="gbtn" id="b-p1-stop"  data-p="p1" data-lin="0"    data-ang="0">■</div>
      <div class="gbtn" id="b-p1-right" data-p="p1" data-lin="0"    data-ang="-1.2">▶</div>
      <div></div>
      <div class="gbtn" id="b-p1-down"  data-p="p1" data-lin="-0.8" data-ang="0">▼</div>
      <div></div>
    </div>
  </div>

  <div class="gpad-wrap">
    <div class="gpad-label p2c">P2 (WASD)</div>
    <div class="gpad">
      <div></div>
      <div class="gbtn" id="b-p2-up"    data-p="p2" data-lin="0.8"  data-ang="0">▲</div>
      <div></div>
      <div class="gbtn" id="b-p2-left"  data-p="p2" data-lin="0"    data-ang="1.2">◀</div>
      <div class="gbtn" id="b-p2-stop"  data-p="p2" data-lin="0"    data-ang="0">■</div>
      <div class="gbtn" id="b-p2-right" data-p="p2" data-lin="0"    data-ang="-1.2">▶</div>
      <div></div>
      <div class="gbtn" id="b-p2-down"  data-p="p2" data-lin="-0.8" data-ang="0">▼</div>
      <div></div>
    </div>
  </div>
</div>

<script src="https://cdn.socket.io/4.7.5/socket.io.min.js"></script>
<script>
const socket = io();
const cv  = document.getElementById('cv');
const ctx = cv.getContext('2d');

// Mapa: 30.72 x 20.48 m, stred (0,0)
const MAP_W = 30.72, MAP_H = 20.48;
const CW = cv.width, CH = cv.height;

// Prevedie svetové súradnice na canvas pixely
function w2c(wx, wy) {
  return [
    (wx + MAP_W/2) / MAP_W * CW,
    (MAP_H/2 - wy) / MAP_H * CH,
  ];
}

// Preconvert meters to canvas pixels (len veľkosť)
function m2px(m) { return m / MAP_W * CW; }

// Mapa ako pozadie
const mapImg = new Image();
mapImg.src = '/map.png';
mapImg.onload = () => drawFrame(lastState);

let lastState = null;
let connected = false;

socket.on('connect',    () => { connected = true;  setStatus('pripojený'); });
socket.on('disconnect', () => { connected = false; setStatus('odpojený'); });

socket.on('state', s => {
  lastState = s;
  updateHUD(s);
  drawFrame(s);
});

function setStatus(t) {
  document.getElementById('status').textContent = t;
}

// ── HUD ──────────────────────────────────────────────────────────────────────
function invStr(paper, plastic, glass) {
  const parts = [];
  if (paper)   parts.push(`📄×${paper}`);
  if (plastic) parts.push(`🧴×${plastic}`);
  if (glass)   parts.push(`🪟×${glass}`);
  return parts.length ? parts.join('  ') : 'prázdny';
}

function updateHUD(s) {
  const g = s.game;
  document.getElementById('timer').textContent =
    g.game_over ? 'KONIEC' : g.time_left.toFixed(1) + ' s';

  if (g.game_over) {
    const wmap = {'p1':'P1 vyhráva!','p2':'P2 vyhráva!','draw':'Remíza!'};
    document.getElementById('winner').textContent = wmap[g.winner] || '';
  } else {
    document.getElementById('winner').textContent = '';
  }

  document.getElementById('p1score').textContent = g.p1_score;
  document.getElementById('p2score').textContent = g.p2_score;
  document.getElementById('p1cap').textContent   = `${g.p1_cap_used}/${g.p1_cap_max}`;
  document.getElementById('p2cap').textContent   = `${g.p2_cap_used}/${g.p2_cap_max}`;
  document.getElementById('p1bar').style.width   = (g.p1_cap_used/g.p1_cap_max*100)+'%';
  document.getElementById('p2bar').style.width   = (g.p2_cap_used/g.p2_cap_max*100)+'%';
  document.getElementById('p1inv').textContent   = invStr(g.p1_paper, g.p1_plastic, g.p1_glass);
  document.getElementById('p2inv').textContent   = invStr(g.p2_paper, g.p2_plastic, g.p2_glass);
}

// ── Canvas ────────────────────────────────────────────────────────────────────
const WASTE_COLORS = {
  paper:   '#ffd700',
  plastic: '#ff9900',
  glass:   '#44ffaa',
};

function drawFrame(s) {
  ctx.clearRect(0, 0, CW, CH);

  // Mapa
  if (mapImg.complete && mapImg.naturalWidth) {
    ctx.globalAlpha = 0.85;
    ctx.drawImage(mapImg, 0, 0, CW, CH);
    ctx.globalAlpha = 1.0;
  } else {
    ctx.fillStyle = '#222';
    ctx.fillRect(0, 0, CW, CH);
  }

  if (!s) return;

  // Stanica (–8, 6), r=0.6
  const [sx, sy] = w2c(-8, 6);
  ctx.beginPath();
  ctx.arc(sx, sy, m2px(0.6), 0, Math.PI*2);
  ctx.strokeStyle = '#00ff88'; ctx.lineWidth = 2;
  ctx.stroke();
  ctx.fillStyle = 'rgba(0,255,136,0.15)';
  ctx.fill();

  // Odpadky
  for (const w of s.wastes) {
    const [wx, wy] = w2c(w.x, w.y);
    const wr = m2px(w.r);
    ctx.beginPath();
    ctx.arc(wx, wy, Math.max(wr, 3), 0, Math.PI*2);
    ctx.fillStyle = WASTE_COLORS[w.type] || '#fff';
    ctx.fill();
    ctx.strokeStyle = '#fff8'; ctx.lineWidth = 1;
    ctx.stroke();
  }

  // Roboty
  drawRobot(s.p1, '#4cb8ff', 'P1');
  drawRobot(s.p2, '#cc66ff', 'P2');
}

function drawRobot(pose, color, label) {
  const [cx, cy] = w2c(pose.x, pose.y);
  const r = m2px(0.20);

  // Telo
  ctx.beginPath();
  ctx.arc(cx, cy, r, 0, Math.PI*2);
  ctx.fillStyle = color + 'bb';
  ctx.fill();
  ctx.strokeStyle = color; ctx.lineWidth = 2;
  ctx.stroke();

  // Smer (šípka)
  const dx =  Math.cos(pose.theta) * r * 1.4;
  const dy = -Math.sin(pose.theta) * r * 1.4;
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.lineTo(cx + dx, cy + dy);
  ctx.strokeStyle = '#fff'; ctx.lineWidth = 2;
  ctx.stroke();

  // Label
  ctx.fillStyle = '#fff';
  ctx.font = `bold ${Math.max(r*0.9,10)}px monospace`;
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(label, cx, cy);
}

// ── Klávesnica ────────────────────────────────────────────────────────────────
const keysDown = new Set();
const LIN = 0.8, ANG = 1.2;

function calcVel(keys, up, down, left, right) {
  let lin = 0, ang = 0;
  if (keys.has(up))    lin =  LIN;
  if (keys.has(down))  lin = -LIN;
  if (keys.has(left))  ang =  ANG;
  if (keys.has(right)) ang = -ANG;
  return {linear: lin, angular: ang};
}

function sendVels() {
  const v1 = calcVel(keysDown,'ArrowUp','ArrowDown','ArrowLeft','ArrowRight');
  const v2 = calcVel(keysDown,'w','s','a','d');
  socket.emit('cmd_vel', {...v1, player:'p1'});
  socket.emit('cmd_vel', {...v2, player:'p2'});
}

document.addEventListener('keydown', e => {
  if (['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' '].includes(e.key)) e.preventDefault();
  keysDown.add(e.key);
  sendVels();
});
document.addEventListener('keyup', e => {
  keysDown.delete(e.key);
  sendVels();
});

// ── Gamepad tlačidlá ──────────────────────────────────────────────────────────
const btnState = {};  // id → bool

function applyBtn(id, active) {
  btnState[id] = active;
  document.getElementById(id).classList.toggle('active', active);

  // Zlúč stav tlačidiel a klávesnice pre každého hráča
  function resolvePlayer(prefix) {
    let lin = 0, ang = 0;
    if (btnState[`b-${prefix}-up`])    lin =  LIN;
    if (btnState[`b-${prefix}-down`])  lin = -LIN;
    if (btnState[`b-${prefix}-left`])  ang =  ANG;
    if (btnState[`b-${prefix}-right`]) ang = -ANG;
    if (btnState[`b-${prefix}-stop`])  { lin = 0; ang = 0; }
    return {linear: lin, angular: ang, player: prefix};
  }

  ['p1','p2'].forEach(p => socket.emit('cmd_vel', resolvePlayer(p)));
}

document.querySelectorAll('.gbtn').forEach(btn => {
  const id = btn.id;
  const activate   = () => applyBtn(id, true);
  const deactivate = () => applyBtn(id, false);
  btn.addEventListener('mousedown',  activate);
  btn.addEventListener('mouseup',    deactivate);
  btn.addEventListener('mouseleave', deactivate);
  btn.addEventListener('touchstart', e => { e.preventDefault(); activate(); });
  btn.addEventListener('touchend',   e => { e.preventDefault(); deactivate(); });
});

function doReset() { socket.emit('reset', {}); }

// Obnova canvasu aj keď neprichádzajú state eventy
setInterval(() => { if (lastState) drawFrame(lastState); }, 200);
</script>
</body>
</html>
"""

# ─── Main ─────────────────────────────────────────────────────────────────────
ros_node: WebBridgeNode | None = None

def ros_thread_fn():
    global ros_node
    rclpy.init(args=None)
    ros_node = WebBridgeNode()
    try:
        rclpy.spin(ros_node)
    except Exception:
        pass
    finally:
        ros_node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    t = threading.Thread(target=ros_thread_fn, daemon=True)
    t.start()
    time.sleep(1.0)  # počkaj kým sa ROS inicializuje
    print("\n========================================")
    print("  OPK Robot Duel – webová aplikácia")
    print("  Otvorte:  http://localhost:5000")
    print("========================================\n")
    socketio.run(app, host="0.0.0.0", port=5000, debug=False, use_reloader=False,
                 allow_unsafe_werkzeug=True)
