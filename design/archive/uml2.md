@startuml
skinparam classAttributeIconSize 0

class Effect {
  +{abstract} void apply(std::shared_ptr<PlayerCharacter> pc)
  +{abstract} void remove(std::shared_ptr<PlayerCharacter> pc)
  +{abstract} bool isTemporary() const
  +virtual ~Effect() = default
}

class BoostAtkEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class WoundAtkEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class BoostDefEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class WoundDefEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class RestoreHealthEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class PoisonHealthEffect {
  +void apply(std::shared_ptr<PlayerCharacter> pc)
  +void remove(std::shared_ptr<PlayerCharacter> pc)
  +bool isTemporary() const
}

class WeatherEffect {
  -int visibilityModifier
  -int movementModifier
  +WeatherEffect(int vis, int mov)
  +void apply(std::shared_ptr<PlayerCharacter> pc) override
  +void remove(std::shared_ptr<PlayerCharacter> pc) override
  +bool isTemporary() const override
}

class RainEffect {
  +RainEffect()
}

class StormEffect {
  +StormEffect()
}

class FogEffect {
  +FogEffect()
}

Effect <|-- WeatherEffect
WeatherEffect <|-- RainEffect
WeatherEffect <|-- StormEffect
WeatherEffect <|-- FogEffect
Effect <|-- BoostAtkEffect
Effect <|-- WoundAtkEffect
Effect <|-- BoostDefEffect
Effect <|-- WoundDefEffect
Effect <|-- RestoreHealthEffect
Effect <|-- PoisonHealthEffect

class EffectManager {
  -static std::shared_ptr<EffectManager> instance
  -static std::once_flag initFlag
  -std::vector<std::unique_ptr<Effect>> effects
  -EffectManager()
  +{static} std::shared_ptr<EffectManager> getInstance()
  +void addEffect(std::unique_ptr<Effect> effect)
  +void clearTemporaryEffects(PlayerCharacter& pc)
  +EffectManager(const EffectManager&) = delete
  +EffectManager& operator=(const EffectManager&) = delete
}

EffectManager --> Effect : manages


@enduml
