#include "global.h"
#include "globaldata.h"
#include "music_util.h"
#include "music.h"
#include "constants/bg_music.h"

// Deleted: sUnknownUnused (write-only, never read) - saves 4 bytes EWRAM
static EWRAM_DATA s16 sSoundEffectCounter1 = {0}; // R=202DE20 | B=213420C
static EWRAM_DATA s16 sSoundEffectCounter2 = {0}; // R=202DE22 | B=2134210
// Deleted: sUnusedCounter (never checked, only set/decremented) - saves 2 bytes EWRAM

void ResetSoundEffectCounters(void)
{
    // Deleted: sUnknownUnused = 0;
    sSoundEffectCounter1 = 0;
    sSoundEffectCounter2 = 0;
    // Deleted: sUnusedCounter = 0;
}

void StopBGMResetSoundEffectCounters(void)
{
    StopBGMusicVSync();
    sSoundEffectCounter1 = 0;
    sSoundEffectCounter2 = 0;
    // Deleted: sUnusedCounter = 0;
}

void StartBGMusic(void)
{
    StartBGMusicVSync();
}

// arm9.bin::020187C0
void UpdateSoundEffectCounters(void)
{
    if (sSoundEffectCounter1 > 0)
        sSoundEffectCounter1--;

    if (sSoundEffectCounter2 > 0)
        sSoundEffectCounter2--;

    // Deleted: if (sUnusedCounter > 0) sUnusedCounter--;
}

void StopAllMusic_1(void)
{
    StopBGMusic();
    StopSound(STOP_FANFARE);
    StopSound(STOP_SOUND_EFFECT);
}

void FadeOutAllMusic(u16 speed)
{
    FadeOutBGM(speed);
    FadeOutFanfareSE(STOP_FANFARE, speed);
    FadeOutFanfareSE(STOP_SOUND_EFFECT, speed);
}

void StartNewBGM_(u16 songIndex)
{
    StartNewBGM(songIndex);
}

void FadeInNewBGM_(u16 songIndex, u16 speed)
{
    FadeInNewBGM(songIndex, speed);
}

void QueueBGM_(u16 songIndex)
{
    QueueBGM(songIndex);
}

void StopBGMusic(void)
{
    StopBGM();
}

void FadeOutBGM_(u16 speed)
{
    FadeOutBGM(speed);
}

bool8 IsEqualtoBGTrack(u16 songIndex)
{
    u32 currBGSong;
    currBGSong = GetCurrentBGSong();

    if (songIndex == STOP_BGM)
        return currBGSong != STOP_BGM;
    return currBGSong == songIndex;
}

void PlaySoundWithVolume(u16 songIndex, u16 volume)
{
    PlayFanfareSE(songIndex, volume);
}

// arm9.bin::020186c8
void PlaySound(u16 songIndex)
{
    PlayFanfareSE(songIndex, MAX_VOLUME);
}

void StopSound(u16 songIndex)
{
    StopFanfareSE(songIndex);
}

void FadeOutSound(u16 songIndex, u16 speed)
{
    FadeOutFanfareSE(songIndex, speed);
}

bool8 IsSoundPlaying(u16 songIndex)
{
    return IsFanfareSEPlaying(songIndex);
}

static const u16 sMenuSoundEffects[] = {302, 303, 303, 301, 304, 306, 307};

void PlayMenuSoundEffect(u32 arrId)
{
    if (sSoundEffectCounter1 > 0)
        return;

    PlayFanfareSE(sMenuSoundEffects[arrId], MAX_VOLUME);
    sSoundEffectCounter1 = 4;
}

void PlayDialogueTextSound(void)
{
    if (sSoundEffectCounter2 > 0)
        return;

    sSoundEffectCounter2 = 3;
    PlayFanfareSE(305, MAX_VOLUME);
}

UNUSED static void sub_8011A2C(u32 a0)
{
    // Deleted: sUnknownUnused = a0;
}
