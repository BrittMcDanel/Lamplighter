#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DEBUG_WHO_SET_IS_MOUSE_OVER_UI 0

#define ITEM_NAME_LAST 12
#define ABILITY_NAME_LAST 7
#define UNIT_NAME_LAST 3
#define TREASURE_CHEST_NAME_LAST 2
#define STATUS_EFFECT_NAME_LAST 2

#define SLOT_OFFSET_X 2
#define SLOT_OFFSET_Y 2
#define SLOT_MARGIN_RIGHT 1
#define SLOT_MARGIN_BOTTOM 1
#define SLOT_DIM 24
#define ITEM_DIM 20
#define STATUS_EFFECT_SLOT_DIM 14
#define BACKGROUND_PADDING 8
#define TOOLTIP_PADDING 4
#define NUM_INVENTORY_SLOTS 42
#define NUM_ABILITY_SLOTS_PER_ROW 21
#define NUM_ABILITY_SLOT_ROWS 5
#define NUM_BATTLE_TURN_ORDER_SLOTS 15

#define HP_MAX 9999
#define STAT_MAX 49
#define ACTION_POINT_MAX 9
#define DAMAGE_STAT_MAX 999
#define RANGE_MAX 60000
#define AOE_MAX 1000
#define CAST_TIME_MAX 20

#define MAX_ABILITY_DAMAGE 9999

#define MOVE_INDEXES_PER_ACTION_POINT 30

#define GAME_ASSERT(condition)                                                 \
  do {                                                                         \
    if (!(condition)) {                                                        \
      cout << "GAME_ASSERT failed. " << __FILE__ << " " << __LINE__ << "\n";   \
      abort();                                                                 \
    }                                                                          \
  } while (0)

enum class ImageName {
  None,
  // shared by every font atlas as they are queried
  // not by image name but by their properties.
  FontAtlas,
  UI,
  Tiles,
  Units,
  Abilities,
  AbilityIcons,
  Misc,
  Icons,
  Buildings,
  Items,
  TreasureChests,
  AbilityTargets,
};

enum class Dir {
  Up,
  Down,
  Left,
  Right,
};

enum class FontParseMode {
  None,
  Chars,
  Kerning,
};

enum class FontStyle {
  Normal,
  Italic,
  Bold,
};

enum class TextAlignment {
  Left,
  Center,
  Right,
};

enum class WordWrap {
  NoWrap,
  Wrap,
};

enum class FontColor {
  None,
  White,
  Black,
  Red,
  WhiteShadow,
  RedShadow,
  RedGradient,
  CyanBlueGradient,
};

enum class CameraMode {
  FreeControl,
  FollowPlayer,
};

enum class CursorType {
  Default,
  Hand,
  Sword,
  SwordInvalid,
  Invalid,
};

enum class Faction {
  Neutral,
  Ally,
  Enemy,
};

enum class UnitAnimState {
  Idle,
  Walk,
  Attack,
  Cast,
  Hit,
  Dead,
};

enum class AbilityTargetDims {
  AOE,
  Range,
};

enum class GameFlag {
  Always = 0,
  TalkedToBob = 1,
  Last = 2,
};

enum class PerformAbilityContext {
  Default,
  Charging,
  Counter,
};

enum class UIPoolHandleType {
  None,
  UISprite,
};

enum class DragGhostType {
  None,
  UISprite,
  TweenableSprite,
  Item,
};

enum class DropType {
  None,
  EquipItem,
  UnequipItem,
  ShopBuyItem,
  ShopBuyItemPutBack,
  ShopSellItem,
  ShopSellItemPutBack,
  EquipAbility,
  BottomNavbarAbilitySwap,
};

enum class TabName {
  None,
  ShopBuy,
  ShopSell,
};

enum class TweenMovingTargetType {
  None,
  Unit,
};

enum class TweenCallbackType {
  None,
  UnitMove,
  SendItemToUnit,
  ItemPickupDisplayComplete,
  BattleAlert,
  BattleText,
  UseAbility,
  UseAbilityTimeout, // js style set timeout for ability usage
};

enum BattleActionType {
  None,
  Move,
  UseAbility,
  EndTurn,
};

enum class BattleState {
  None,
  Starting,
  Active,
  AlliesWon,
  EnemiesWon,
};

enum class AbilityType {
  None,
  Damage,
  Heal,
  StatusEffect,
};

enum class AbilityElement {
  None,
  Physical,
  Fire,
  Water,
  Earth,
  Lightning,
  Light,
  Shadow,
};

enum class ShaderName {
  None,
  Default,
};

enum class ItemType {
  None,
  Money,
  Misc,
  Useable,
  PrimaryHand,
  SecondaryHand,
  Armor,
  Head,
  Boots,
};

enum class StatusEffectType {
  Buff,
  Nerf,
};

enum class TreasureChestName {
  None = 0,
  Small = 1,
  Last = TREASURE_CHEST_NAME_LAST,
};

enum class UnitName {
  None = 0,
  Ally = 1,
  Enemy = 2,
  Last = UNIT_NAME_LAST,
};

enum class AbilityName {
  None = 0,
  Fire = 1,
  FireBall = 2,
  SwordSlash = 3,
  CounterAttack = 4,
  Bullet = 5,
  Resolve = 6,
  Last = ABILITY_NAME_LAST,
};

enum class StatusEffectName {
  None = 0,
  Resolve = 1,
  Last = STATUS_EFFECT_NAME_LAST,
};

enum class ItemName {
  None = 0,
  Coin = 1,
  Cash = 2,
  SlimeJelly = 3,
  FurPelt = 4,
  Stump = 5,
  Mushroom = 6,
  BatWing = 7,
  Apple = 8,
  IronArmor = 9,
  IronHelmet = 10,
  IronSword = 11,
  Last = ITEM_NAME_LAST,
};

#endif // CONSTANTS_H
