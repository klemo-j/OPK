#!/usr/bin/env python3
"""
Duel teleop - globalna klavesnica cez pynput.
P1 (sipky):  -> /p1/robot/cmd_vel
P2 (WASD):   -> /p2/robot/cmd_vel

Vyhody pynput:
- Kazda klavesa nezavisle (drz sipku + W naraz)
- Funguje aj ked okno nema fokus (ale OS focus stale potrebny)
- Spravne sleduje pressed/released stav
"""
import time
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from pynput import keyboard

LIN = 0.8
ANG = 1.2

HELP = """
============================================
  DUEL TELEOP (pynput - paralelne klavesy)
============================================
  P1 (sipky):       P2 (WASD):
    UP    dopredu     W   dopredu
    DOWN  dozadu      S   dozadu
    LEFT  dolava      A   dolava
    RIGHT doprava     D   doprava

  Drz klavesu = jazdi. Pusti = stop (okamzite).
  Mozes drzat viac klaves naraz!

  ESC = quit
============================================
"""


def twist(lin, ang):
    t = Twist()
    t.linear.x = lin
    t.angular.z = ang
    return t


class DuelTeleop(Node):
    def __init__(self):
        super().__init__('duel_teleop')
        self.p1_pub = self.create_publisher(Twist, '/p1/robot/cmd_vel', 10)
        self.p2_pub = self.create_publisher(Twist, '/p2/robot/cmd_vel', 10)

        # Stav stlacenych klaves
        self.pressed = set()

        # Publikujeme 50x za sekundu
        self.timer = self.create_timer(0.02, self.tick)

        # Spusti listener klavesnice (v pozadi, vlastne vlakno)
        self.listener = keyboard.Listener(
            on_press=self.on_press,
            on_release=self.on_release)
        self.listener.start()

        print(HELP)

    def on_press(self, key):
        try:
            # Specialne klavesy (sipky)
            if key == keyboard.Key.up:    self.pressed.add('UP')
            elif key == keyboard.Key.down:  self.pressed.add('DOWN')
            elif key == keyboard.Key.left:  self.pressed.add('LEFT')
            elif key == keyboard.Key.right: self.pressed.add('RIGHT')
            elif key == keyboard.Key.esc:
                rclpy.shutdown()
                return False
            elif hasattr(key, 'char') and key.char:
                self.pressed.add(key.char.lower())
        except AttributeError:
            pass

    def on_release(self, key):
        try:
            if key == keyboard.Key.up:    self.pressed.discard('UP')
            elif key == keyboard.Key.down:  self.pressed.discard('DOWN')
            elif key == keyboard.Key.left:  self.pressed.discard('LEFT')
            elif key == keyboard.Key.right: self.pressed.discard('RIGHT')
            elif hasattr(key, 'char') and key.char:
                self.pressed.discard(key.char.lower())
        except AttributeError:
            pass

    def tick(self):
        # P1 - sipky
        p1_lin = 0.0
        p1_ang = 0.0
        if 'UP' in self.pressed:    p1_lin = LIN
        if 'DOWN' in self.pressed:  p1_lin = -LIN
        if 'LEFT' in self.pressed:  p1_ang = ANG
        if 'RIGHT' in self.pressed: p1_ang = -ANG

        # P2 - WASD
        p2_lin = 0.0
        p2_ang = 0.0
        if 'w' in self.pressed: p2_lin = LIN
        if 's' in self.pressed: p2_lin = -LIN
        if 'a' in self.pressed: p2_ang = ANG
        if 'd' in self.pressed: p2_ang = -ANG

        self.p1_pub.publish(twist(p1_lin, p1_ang))
        self.p2_pub.publish(twist(p2_lin, p2_ang))



def main():
    rclpy.init()
    node = DuelTeleop()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.p1_pub.publish(Twist())
        node.p2_pub.publish(Twist())
        node.listener.stop()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
