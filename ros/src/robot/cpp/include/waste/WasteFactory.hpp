#pragma once

#include "Waste.hpp"
#include "types/Geometry.hpp"
#include <memory>
#include <random>
#include <string>

namespace waste {

// ─────────────────────────────────────────────────────────────────────────────
// Design Pattern: Factory Method
//
// Abstraktná továreň definuje rozhranie pre tvorbu odpadkov.
// Každá konkrétna továreň vytvára špecifický typ odpadu.
// Generátor odpadkov (WasteGenerator) používa továrne cez polymorfizmus.
// ─────────────────────────────────────────────────────────────────────────────

// Abstraktná továreň (Factory Method)
class WasteFactory {
public:
    virtual ~WasteFactory() = default;

    // Továrenská metóda – tvorí odpad na danej pozícii s daným polomerom
    virtual std::unique_ptr<Waste> create(
        const geometry::Point2d& position,
        double radius
    ) const = 0;

    // Každá továreň pozná svoj typ
    virtual WasteType getType() const = 0;
    virtual std::string getTypeName() const = 0;
};

// ─── Konkrétne továrne ───────────────────────────────────────────────────────

class PaperWasteFactory : public WasteFactory {
public:
    std::unique_ptr<Waste> create(
        const geometry::Point2d& position, double radius
    ) const override {
        return std::make_unique<PaperWaste>(position, radius);
    }
    WasteType   getType()     const override { return WasteType::PAPER; }
    std::string getTypeName() const override { return "paper"; }
};

class PlasticWasteFactory : public WasteFactory {
public:
    std::unique_ptr<Waste> create(
        const geometry::Point2d& position, double radius
    ) const override {
        return std::make_unique<PlasticWaste>(position, radius);
    }
    WasteType   getType()     const override { return WasteType::PLASTIC; }
    std::string getTypeName() const override { return "plastic"; }
};

class GlassWasteFactory : public WasteFactory {
public:
    std::unique_ptr<Waste> create(
        const geometry::Point2d& position, double radius
    ) const override {
        return std::make_unique<GlassWaste>(position, radius);
    }
    WasteType   getType()     const override { return WasteType::GLASS; }
    std::string getTypeName() const override { return "glass"; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Konfigurácia generátora odpadkov
// ─────────────────────────────────────────────────────────────────────────────
struct WasteGeneratorConfig {
    double radius_min = 0.10;   // minimálny polomer odpadku [m]
    double radius_max = 0.30;   // maximálny polomer odpadku [m]
    double map_width  = 20.0;   // šírka mapy [m] – pre random pozíciu
    double map_height = 20.0;   // výška mapy [m]
};

// ─────────────────────────────────────────────────────────────────────────────
// Generátor odpadkov
//
// Používa polymorfné továrne – pridanie nového typu odpadu nevyžaduje
// zmenu tejto triedy (Open/Closed Principle).
// ─────────────────────────────────────────────────────────────────────────────
class WasteGenerator {
public:
    explicit WasteGenerator(const WasteGeneratorConfig& config, unsigned int seed = 42)
        : config_(config), rng_(seed)
    {
        // Registrujeme všetky podporované typy
        factories_.push_back(std::make_unique<PaperWasteFactory>());
        factories_.push_back(std::make_unique<PlasticWasteFactory>());
        factories_.push_back(std::make_unique<GlassWasteFactory>());
    }

    // Vygeneruje jeden náhodný odpad
    std::unique_ptr<Waste> generateRandom() {
        if (factories_.empty()) {
            throw game::GameException("WasteGenerator: žiadne továrne nie sú registrované");
        }

        // Náhodný výber továrne (= typu odpadu)
        std::uniform_int_distribution<size_t> type_dist(0, factories_.size() - 1);
        const auto& factory = factories_[type_dist(rng_)];

        // Náhodná pozícia v rámci mapy
        std::uniform_real_distribution<double> x_dist(1.0, config_.map_width  - 1.0);
        std::uniform_real_distribution<double> y_dist(1.0, config_.map_height - 1.0);
        geometry::Point2d pos{ x_dist(rng_), y_dist(rng_) };

        // Náhodný polomer v zadanom rozsahu
        std::uniform_real_distribution<double> r_dist(config_.radius_min, config_.radius_max);
        double radius = r_dist(rng_);

        return factory->create(pos, radius);
    }

    // Vygeneruje n odpadkov naraz
    std::vector<std::unique_ptr<Waste>> generateBatch(size_t count) {
        std::vector<std::unique_ptr<Waste>> batch;
        batch.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            batch.push_back(generateRandom());
        }
        return batch;
    }

    // Vygeneruje odpad konkrétneho typu (pre testovanie / špeciálne módy)
    std::unique_ptr<Waste> generateOfType(
        WasteType type,
        const geometry::Point2d& position,
        double radius
    ) {
        for (const auto& factory : factories_) {
            if (factory->getType() == type) {
                return factory->create(position, radius);
            }
        }
        throw game::GameException(
            "WasteGenerator: typ '" + wasteTypeToString(type) + "' nie je registrovaný"
        );
    }

    size_t getFactoryCount() const { return factories_.size(); }

private:
    WasteGeneratorConfig config_;
    std::mt19937 rng_;
    std::vector<std::unique_ptr<WasteFactory>> factories_;
};

} // namespace waste
