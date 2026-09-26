#include "pe/Util/Localization.h"
#include "al/Library/String/StringUtil.h"
#include "pe/Menu/UserConfig.h"

namespace pe {

struct MsgEntry {
    const char* msgId;
    const char* msgContents[4];
};

constexpr MsgEntry sLocalizationData[] {
    { "false", { "No", "No", "Nein", "Non" } },
    { "true", { "Yes", "Yes", "Ja", "Oui" } },

    // Categories
    { "options", { "Options", "オプション", "Optionen", "Options" } },
    { "stage", { "Stages", "ステージ", "Level", "Niveaux" } },
    { "misc", { "Misc", "その他", "Verschiedenes", "Divers" } },
    { "timer", { "Timer", "タイマー", "Timer", "Chronomètre" } },
    { "inputdisplay", { "Input Display", "入力表示", "Input Display", "Affichage Manette" } },
    { "keybinds", { "Keybinds", "キー設定", "Tastenkürzel", "Raccourcis" } },
    { "info", { "Info", "情報", "Info", "Informations" } },
    { "settings", { "Settings", "詳細設定", "Einstellungen", "Paramètres" } },

    // Actions
    { "action0", { "None", "なし", "Keine", "Aucun" } },
    { "action1", { "Restart Scene", "Reload Scene", "Szenen-Neustart", "Recharger Scène" } },
    { "action2", { "Previous Scene", "Previous Scene", "Vorherige Szene", "Scène Précédente" } },
    { "action3", { "Save Position", "位置をセーブ", "Position Speichern", "Sauvegarder Position" } },
    { "action4", { "Load Position", "位置をロード", "Position Laden", "Recharger Position" } },
    { "action5", { "Start Timer", "タイマースタート", "Timer Starten", "Démarrer Chrono" } },
    { "action6", { "Stop Timer", "タイマーストップ", "Timer Beenden", "Arrêter Chrono" } },
    { "action7", { "Reset Timer", "タイマーリセット", "Timer-Reset", "Réinitialiser Chrono" } },
    { "action8", { "Next Wiggler Pattern", "メカハナ 次の乱数", "Nächstes Wiggler-Muster", "Pattern Wiggler Suivant" } },
    { "action9", { "Previous Wiggler Pattern", "メカハナ 前の乱数", "Vorheriges Wiggler-Muster", "Pattern Wiggler Précédent" } },
    { "action10", { "Life-Up Heart", "ライフUPハート", "1-UP Herz", "Cœur Max (+1 Cœur)" } },
    { "action11", { "Heal Mario", "Heal Mario", "Mario Heilen", "Soigner Mario" } },
    { "action12", { "Add 1000 Coins", "+1000コイン", "+1000 Münzen", "+1000 Pièces" } },
    { "action13", { "Subtract 1000 Coins", "-1000コイン", "-1000 Münzen", "-1000 Pièces" } },
    { "action14", { "Warp to last Checkpoint", "前の旗に飛ぶ", "Zum letzten Checkpoint warpen", "Warp Dernier Checkpoint" } },
    { "action15", { "Kill Mario", "Kill Mario", "Töte mario", "Tuer Mario" } },
    { "action16", { "Toggle Noclip", "Noclip切替", "Noclip umschalten", "Basculer Noclip" } },

    // Refresh Settings
    { "moonrefresh", { "Moon Refresh", "Moon Refresh", "Monde erneuern", "Rafraîchir Lunes" } },
    { "greymoonrefresh", { "Gray Moon Refresh", "Gray Moon Refresh", "Graue Monde erneuern", "Rafraîchir Lunes Grises" } },
    { "purplecoinsrefresh", { "Purple Coins Refresh", "紫コイン復活", "Lila Münzen erneuern", "Rafraîchir Pièces Violettes" } },
    { "doorsrefresh", { "Doors Refresh", "ドア復活", "Türen erneuern", "Rafraîchir Portes" } },
    { "shardsrefresh", { "Moon Shards Refresh", "ムーンチップ復活", "Mondscherben erneuern", "Rafraîchir Éclats de Lune" } },
    { "kingdomenterrefresh", { "Kingdom Enter Cutscenes Refresh", "国入場ムービー復活", "Lande-Cutscenes erneuern", "Rafraîchir Entrée Royaume" } },
    { "warptextrefresh", { "Warp Text Refresh", "ワープテキスト復活", "Warp-Texte erneuern", "Rafraîchir Texte de Warp" } },
    { "seedsrefresh", { "Seeds Refresh", "タネ復活", "Samen erneuern", "Rafraîchir Graines" } },
    { "cutscenerefresh", { "Cutscene Refresh", "キノピオ等テキスト復活", "NPC-Texte erneuern", "Rafraîchir Dialogue PNJ" } },

    // Gameplay & Disable Toggles
    { "seedsgrowing", { "Seeds Growing", "タネ即座に成長", "Samen wachsen lassen", "Pousse Immédiate Graines" } },
    { "alwaysmanualcutscene", { "Always Manual Skip Cutscene", "イベントスキップ有効化", "Cutscenes manuell überspringen", "Passer Cinématiques Manuellement" } },
    { "alwayscheckpoints", { "Always Allow Checkpoints", "常に中間ワープ可能", "Checkpoints immer erlauben", "Toujours Autoriser Checkpoints" } },
    { "disablemoonlock", { "Disable Moon Lock", "ムーンロック解除", "Mondsperre deaktivieren", "Désactiver Verrou de Lunes" } },
    { "disableautosave", { "Autosave", "オートセーブ", "Automatisches Speichern", "Sauvegarde Automatique" } },
    { "disabletppuppet", { "Disable Teleport Puppet", "TP時操作不能解除", "Teleport-Puppet deaktivieren", "Sans Marionnette au TP" } },
    { "disablemusic", { "Disable Music", "BGM停止", "Musik deaktivieren", "Désactiver Musique" } },
    { "nodamage", { "No Damage", "無敵 (ノーダメージ)", "Kein Schaden", "Invincible (Sans Dégât)" } },
    { "nocheckpointtouch", { "No Checkpoint Touch", "中間ポイント無効", "Keine Checkpoint-Berührung", "Pas de Touche Checkpoint" } },
    { "skipcloud", { "Skip Cloud", "クラスタスキップ", "Wolkenland überspringen", "Passer Pays des Nuages" } },
    { "skipbroodals", { "Skip Broodals Fight", "ブルーダルズスキップ", "Broodals-Kampf überspringen", "Passer Combat Broodals" } },
    { "allcheckpoints", { "Enable All Checkpoints", "全中間フラグ有効化", "Alle Checkpoints aktivieren", "Débloquer Tous Checkpoints" } },
    { "luigipixels", { "Enable Luigi Pixels Hint Art", "ドットルイージヒントアート", "Luigi-Pixel Hint Art aktivieren", "Activer Indices Pixels Luigi" } },
    { "spawnallhintart", { "Spawn All Hint Art Moons", "全ヒントアートムーン出現", "Alle Hint-Art-Monde spawnen", "Faire Apparaître Toutes Lunes Art" } },

    // Misc
    { "saveposition", { "Save Position", "位置セーブ", "Position Speichern", "Sauvegarder Position" } },
    { "loadposition", { "Load Position", "位置ロード", "Position Laden", "Recharger Position" } },
    { "killmario", { "Kill Mario", "マリオを倒す", "Mario töten", "Tuer Mario" } },
    { "damagemario", { "Damage Mario", "ダメージを与える", "Mario verletzen", "Blesser Mario" } },
    { "lifeup", { "Life-Up Heart", "ライフUPハート", "1-UP Herz", "Cœur Max (+1 Cœur)" } },
    { "healmario", { "Heal Mario", "全回復", "Mario heilen", "Soigner Mario" } },
    { "add1000", { "Add 1000 Coins", "+1000コイン", "+1000 Münzen", "+1000 Pièces" } },
    { "sub1000", { "Subtract 1000 Coins", "-1000コイン", "-1000 Münzen", "-1000 Pièces" } },
    { "killscene", { "Kill Scene", "ステージ再読込", "Szene neu laden", "Recharger Scène" } },
    { "prevscene", { "Previous Scene", "前のステージへ", "Vorherige Szene", "Scène Précédente" } },
    { "warplastcheckpoint", { "Warp to Last Checkpoint", "直前の中間へ飛ぶ", "Zum letzten Checkpoint", "Warp Dernier Checkpoint" } },
    { "noclip", { "No Clip", "壁抜け (Noclip)", "Noclip (Durch Wände)", "Traverser les Murs (Noclip)" } },
    { "wigglerpattern", { "Wiggler Pattern", "メカハナ乱数", "Wiggler-Muster", "Pattern Wiggler" } },
    { "moonname", { "Moon Name", "ムーン表示名", "Mondname", "Nom de la Lune" } },
    { "outfitbody", { "Costume Body", "服装", "Kostüm", "Costume Mario" } },
    { "outfitcap", { "Costume Cap", "帽子", "Mütze", "Chapeau Mario" } },
    { "setoutfit", { "Apply Outfit (Reloads)", "服装変更 (再読込)", "Kostüm anwenden", "Appliquer Costume (Recharge)" } },

    // Timer
    { "autokingdom", { "Auto Kingdom Timer", "国ごと自動タイマー", "Auto-Kingdom-Timer", "Chrono Auto par Royaume" } },
    { "starttimer", { "Start Timer", "タイマースタート", "Timer Start", "Démarrer Chrono" } },
    { "stoptimer", { "Stop Timer", "タイマーストップ", "Timer Stop", "Arrêter Chrono" } },
    { "resettimer", { "Reset Timer", "タイマーリセット", "Timer Reset", "Réinitialiser Chrono" } },
    { "timersplit", { "Split on Moon", "ムーン取得でスプリット", "Split bei Mond", "Split aux Lunes" } },
    { "fontsize", { "Font Size", "フォントサイズ", "Schriftgröße", "Taille Police" } },
    { "position", { "Position", "位置", "Position", "Position" } },

    // Input Display
    { "2p_mode", { "2P Mode", "2Pモード", "2P-Modus", "Mode 2 Joueurs (2P)" } },

    // Keybinds
    { "dpadup", { "D-Pad Up", "十字キー 上", "D-Pad Oben", "Flèche Haut" } },
    { "dpaddown", { "D-Pad Down", "十字キー 下", "D-Pad Unten", "Flèche Bas" } },
    { "dpadright", { "D-Pad Right", "十字キー 右", "D-Pad Rechts", "Flèche Droite" } },
    { "dpadleft", { "D-Pad Left", "十字キー 左", "D-Pad Links", "Flèche Gauche" } },
    { "rsl", { "RStick + Left", "Rスティック + 左", "R-Stick + Links", "R-Stick + Gauche" } },
    { "rsr", { "RStick + Right", "Rスティック + 右", "R-Stick + Rechts", "R-Stick + Droite" } },
    { "zlzr", { "ZL + ZR", "ZL + ZR", "ZL + ZR", "ZL + ZR" } },
    { "wheel1", { "Wheel 1 (Up)", "ホイール 1 (上)", "Rad 1 (Oben)", "Roue 1 (Haut)" } },
    { "wheel2", { "Wheel 2 (Up-Right)", "ホイール 2 (右上)", "Rad 2 (Oben-Rechts)", "Roue 2 (Haut-Droite)" } },
    { "wheel3", { "Wheel 3 (Right)", "ホイール 3 (右)", "Rad 3 (Rechts)", "Roue 3 (Droite)" } },
    { "wheel4", { "Wheel 4 (Down-Right)", "ホイール 4 (右下)", "Rad 4 (Unten-Rechts)", "Roue 4 (Bas-Droite)" } },
    { "wheel5", { "Wheel 5 (Down)", "ホイール 5 (下)", "Rad 5 (Unten)", "Roue 5 (Bas)" } },
    { "wheel6", { "Wheel 6 (Down-Left)", "ホイール 6 (左下)", "Rad 6 (Unten-Links)", "Roue 6 (Bas-Gauche)" } },
    { "wheel7", { "Wheel 7 (Left)", "ホイール 7 (左)", "Rad 7 (Links)", "Roue 7 (Gauche)" } },
    { "wheel8", { "Wheel 8 (Up-Left)", "ホイール 8 (左上)", "Rad 8 (Oben-Links)", "Roue 8 (Haut-Gauche)" } },

    // Info
    { "infowindow", { "Window Info", "ゲーム統計ウィンドウ", "Info-Fenster", "Fenêtre Statistiques" } },
    { "playerinfo", { "Player Info", "プレイヤー情報 (座標・速度)", "Spieler-Info", "Infos Joueur (Coords, Vitesse)" } },
    { "resetinfopos", { "Reset Position", "位置リセット", "Position zurücksetzen", "Réinitialiser Position" } },

    // Settings
    { "wheeltime", { "Wheel Time (frames)", "ホイール受付時間", "Rad-Haltezeit (Frames)", "Délai Détection Roue" } },
    { "wheelstick", { "Wheel Stick", "ホイール用スティック", "Rad-Stick", "Stick de la Roue" } },
    { "left", { "Left Stick", "Lスティック", "Linker Stick", "Stick Gauche" } },
    { "right", { "Right Stick", "Rスティック", "Rechter Stick", "Stick Droit" } },

    // Stage
    { "scenario", { "Scenario", "シナリオ", "Szenario", "Scénario" } },
    { "go", { "Warp Now", "ワープ実行", "Warpen", "Téléporter" } },

    // Ghost
    { "ghostreplay", { "Ghost Replay", "ゴースト再生", "Geist-Wiedergabe", "Replay Fantôme" } },
    { "pboverlay", { "PB HUD Overlay", "PB HUD 表示", "PB-HUD-Overlay", "Overlay HUD Record (PB)" } },
    { "ghostalpha", { "Ghost Opacity", "ゴースト透明度", "Geist-Deckkraft", "Opacité Fantôme" } },
    { "resetpb", { "Reset Current PB", "現在のPBを削除", "Aktuellen PB löschen", "Réinitialiser Record PB" } }
};

const char* getLocalizedString(const char* msg)
{
    auto* cfg = getConfig();
    int langIdx = cfg ? (int)cfg->currentLanguage : 0;
    if (langIdx < 0 || langIdx > 3) langIdx = 0;

    for (const MsgEntry& entry : sLocalizationData) {
        if (al::isEqualString(msg, entry.msgId))
            return entry.msgContents[langIdx];
    }

    return msg ? msg : "NULL";
}

} // namespace pe
