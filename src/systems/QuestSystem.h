#pragma once

#include "../events/EventBus.h"
#include "../events/GameEvents.h"

#include <string>
#include <vector>
#include <cstdint>
#include <ctime>

// ==============================================================================
// QuestType — what counter the quest tracks
//
// ==============================================================================
enum class QuestType
{
    KillEnemies,    // Kill N enemies
    KillBosses,     // Kill N bosses
    EarnCoins,      // Earn N coins total
    SpendCoins,     // Spend N coins
    CraftItems,     // Craft N items
    CompleteStages, // Complete N stages
    UsePotion,      // Use N potions
};

// ==============================================================================
// QuestPeriod — when the quest resets
//
// ==============================================================================
enum class QuestPeriod
{
    Daily,  // Resets every day at midnight
    Weekly, // Resets every Monday at midnight
};

// ==============================================================================
// Quest — a single task definition and its current progress
//
// ==============================================================================
struct Quest
{
    uint32_t id;
    std::string name;
    std::string description;
    QuestType type;
    QuestPeriod period;

    int goal;         // Target count to complete the quest
    int currentCount; // Current progress toward goal
    int coinReward;   // Coins awarded on completion

    bool isCompleted = false;
    bool isRewarded = false; // Player has collected the reward

    // Returns 0.0 to 1.0 progress
    float GetProgress() const
    {
        if (goal <= 0)
            return 1.0f;
        return std::min(1.0f, static_cast<float>(currentCount) / goal);
    }
};

// ==============================================================================
// QuestSystem
//
// Subscribes to EventBus events and automatically advances quest progress
// whenever relevant game events fire.
//
// ==============================================================================
class QuestSystem
{
public:
    QuestSystem(EventBus &eventBus);
    ~QuestSystem();

    // Init — subscribes to EventBus and registers default quests.
    // Call once after EventBus is created.
    void Init();

    // Shutdown — unsubscribes all listeners.
    void Shutdown();

    // Update — checks if daily/weekly reset is needed.
    // Call once per frame from Game::Update() (cheap — just timestamp compare).
    void Update();

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    const std::vector<Quest> &GetDailyQuests() const { return m_dailyQuests; }
    const std::vector<Quest> &GetWeeklyQuests() const { return m_weeklyQuests; }

    // GetQuest — find a quest by ID across both lists
    const Quest *GetQuest(uint32_t questId) const;

    // -----------------------------------------------------------------------
    // Reward collection
    // -----------------------------------------------------------------------

    // ClaimReward — marks quest as rewarded and fires QuestRewardClaimed.
    // Returns coin reward amount (0 if quest not completed or already claimed).
    int ClaimReward(uint32_t questId);

    // -----------------------------------------------------------------------
    // Dungeon daily limits
    // -----------------------------------------------------------------------

    static constexpr int DUNGEON_MAX_ATTEMPTS = 3; // per day

    int GetDungeonAttemptsRemaining(int dungeonId) const;
    bool UseAttempt(int dungeonId);
    void ResetDungeonAttempts(); // Called on daily reset

private:
    EventBus &m_eventBus;

    std::vector<Quest> m_dailyQuests;
    std::vector<Quest> m_weeklyQuests;

    // Stored as time_t (seconds since epoch) — compared against current time
    time_t m_lastDailyReset = 0;
    time_t m_lastWeeklyReset = 0;

    // Dungeon attempts: dungeonId → attempts used today
    std::unordered_map<int, int> m_dungeonAttemptsUsed;

    // Event listener IDs
    ListenerID m_enemyDiedListenerId = 0;
    ListenerID m_bossDiedListenerId = 0;
    ListenerID m_coinGainedListenerId = 0;
    ListenerID m_coinSpentListenerId = 0;
    ListenerID m_itemCraftedListenerId = 0;
    ListenerID m_stageClearedListenerId = 0;
    ListenerID m_potionUsedListenerId = 0;
    ListenerID m_dungeonAttemptListenerId = 0;

    // Quest registration
    void RegisterDailyQuests();
    void RegisterWeeklyQuests();

    // Progress advancement — called by event handlers
    void AdvanceQuests(QuestType type, int amount = 1);

    // Reset logic
    void CheckResets();
    void ResetDailyQuests();
    void ResetWeeklyQuests();

    bool IsNewDay() const;
    bool IsNewWeek() const;

    // Quest lookup (non-const for modification)
    Quest *FindQuest(uint32_t questId);
};