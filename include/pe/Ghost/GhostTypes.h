#pragma once

#include "basis/seadTypes.h"
#include "math/seadQuat.h"
#include "math/seadVector.h"

namespace pe {

// Identifiants des Royaumes (Kingdoms) de Super Mario Odyssey
enum class KingdomId : s32 {
    Cap = 0,
    Cascade = 1,
    Sand = 2,
    Lake = 3,
    Wooded = 4,
    Cloud = 5,
    Lost = 6,
    Metro = 7,
    Snow = 8,
    Seaside = 9,
    Luncheon = 10,
    Ruined = 11,
    Bowser = 12,
    Moon = 13,
    DarkSide = 14,
    DarkerSide = 15,
    Mushroom = 16,
    Unknown = -1
};

// Identifiants des segments / sous-sections d'IL
enum class SegmentId : s32 {
    FullKingdom = 0,
    CapEnd = 1,
    CascadeOdyssey = 2,
    SandOdyssey = 3,
    LakeOdyssey = 4,
    WoodedOdyssey = 5,
    CloudSkip = 6,
    LostOdyssey = 7,
    MetroOdyssey = 8,
    SnowOdyssey = 9,
    SeasideOdyssey = 10,
    LuncheonOdyssey = 11,
    RuinedOdyssey = 12,
    BowserBunnies = 13,    // "Bunnies" (Rabbit Ridge / Dark Side / Bowser)
    BowserOdyssey = 14,    // "Bowsers"
    MoonEnd = 15,          // "Moon"
    Custom = 99
};

// Types d'événements de déclenchement (Autosplitter triggers)
enum class TriggerType : u8 {
    None = 0,
    StartButton = 1,
    MoonStory = 2,
    BlackScreen = 3,
    Odyssey = 4,
    MoonEnd = 5,
    CapEnd = 6,
    CutsceneSkip = 7
};

// Structure d'une frame d'enregistrement synchronisée
struct ReplayFrame {
    u32 mStep = 0;                     // Numéro de frame absolu depuis le début de l'IL
    u32 mPlayerAnimHash = 0;           // Hash CRC32 de l'animation Mario en cours
    u32 mCapAnimHash = 0;              // Hash CRC32 de l'animation Cappy
    float mBlendWeights[6] = {0};      // Poids de blending squelettique
    sead::Vector3f mPlayerTrans = sead::Vector3f::zero;
    sead::Quatf mPlayerQuat = sead::Quatf::unit;
    sead::Vector3f mCapTrans = sead::Vector3f::zero;
    sead::Quatf mCapQuat = sead::Quatf::unit;
    sead::Vector3f mCapJoint = sead::Vector3f::zero;
    float mCapSkew = 0.0f;
    
    // États et drapeaux
    bool mIsCapVisible = true;
    bool mIs2D = false;
    bool mIsHack = false;
    
    // Inputs manette pour synchronisation complète
    u32 mButtons = 0;
    sead::Vector2f mLeftStick = sead::Vector2f::zero;
    sead::Vector2f mRightStick = sead::Vector2f::zero;
};

// En-tête du fichier d'enregistrement de Replay (.bin)
struct GhostHeader {
    static constexpr u32 MAGIC = 0x54534847; // 'GHST'
    static constexpr u32 CURRENT_VERSION = 1;

    u32 mMagic = MAGIC;
    u32 mVersion = CURRENT_VERSION;
    KingdomId mKingdomId = KingdomId::Unknown;
    SegmentId mSegmentId = SegmentId::FullKingdom;
    u64 mProfileId = 0;                // Identifiant du profil Switch actif
    s64 mTotalTicks = 0;               // Temps total mesuré en ticks système
    u32 mTotalFrames = 0;              // Nombre de frames enregistrées
    char mStageName[64] = {0};         // Nom interne du niveau de départ
    s32 mScenarioNo = -1;              // Numéro de scénario
    char mSegmentName[64] = {0};       // Nom lisible du segment (ex: "Bunnies", "Bowsers")
};

// Données de score / PB persistées pour un segment
struct PBRecord {
    KingdomId mKingdomId = KingdomId::Unknown;
    SegmentId mSegmentId = SegmentId::FullKingdom;
    char mSegmentName[64] = {0};
    s64 mBestTicks = 0;                // Temps record en ticks (0 = pas encore de PB)
    u32 mBestFrames = 0;
    u64 mAchievedTimestamp = 0;        // Horodatage POSIX de la réalisation
    bool mHasGhostReplay = false;      // Indique si le fichier .bin correspondant existe
};

} // namespace pe
