#include "QuestSystem.h"

#include <iostream>
#include <algorithm>

QuestSystem::QuestSystem(EventBus &eventBus)
    : m_eventBus(eventBus)
{
}

QuestSystem::~QuestSystem()
{
    Shutdown();
}

// ==============================================================================
// Init / Shutdown
// ==============================================================================

void QuestSystem::Init()
{
    RegisterDailyQuests();
    RegisterWeeklyQuests();

    // Record reset timestamps as now — quests start fresh
    m_lastDailyReset = std::time(nullptr);
    m_lastWeeklyReset = std::time(nullptr);

    // Subscribe to all events that advance quest progress.
    // The lambda just extracts the relevant count and calls AdvanceQuests().

    m_enemyDiedListenerId = m_eventBus.Subscribe(EventType::EnemyDied,
                                                 [this](const Event &)
                                                 {
                                                     AdvanceQuests(QuestType::KillEnemies, 1);
                                                 });

    m_bossDiedListenerId = m_eventBus.Subscribe(EventType::BossDied,
                                                [this](const Event &)
                                                {
                                                    AdvanceQuests(QuestType::KillBosses, 1);
                                                    AdvanceQuests(QuestType::CompleteStages, 1);
                                                });

    m_coinGainedListenerId = m_eventBus.Subscribe(EventType::CoinGained,
                                                  [this](const Event &e)
                                                  {
                                                      auto &data = std::get<CoinGainedEvent>(e.data);
                                                      AdvanceQuests(QuestType::EarnCoins, data.amount);
                                                  });

    m_coinSpentListenerId = m_eventBus.Subscribe(EventType::CoinSpent,
                                                 [this](const Event &e)
                                                 {
                                                     auto &data = std::get<CoinSpentEvent>(e.data);
                                                     AdvanceQuests(QuestType::SpendCoins, data.amount);
                                                 });

    m_itemCraftedListenerId = m_eventBus.Subscribe(EventType::ItemCrafted,
                                                   [this](const Event &)
                                                   {
                                                       AdvanceQuests(QuestType::CraftItems, 1);
                                                   });

    m_potionUsedListenerId = m_eventBus.Subscribe(EventType::PotionUsed,
                                                  [this](const Event &)
                                                  {
                                                      AdvanceQuests(QuestType::UsePotion, 1);
                                                  });

    m_dungeonAttemptListenerId = m_eventBus.Subscribe(EventType::DungeonAttemptUsed,
                                                      [this](const Event &e)
                                                      {
                                                          auto &data = std::get<DungeonAttemptUsedEvent>(e.data);
                                                          m_dungeonAttemptsUsed[data.dungeonId]++;
                                                      });

    std::cout << "[QuestSystem] Initialized with "
              << m_dailyQuests.size() << " daily and "
              << m_weeklyQuests.size() << " weekly quests.\n";
}

void QuestSystem::Shutdown()
{
    m_eventBus.Unsubscribe(EventType::EnemyDied, m_enemyDiedListenerId);
    m_eventBus.Unsubscribe(EventType::BossDied, m_bossDiedListenerId);
    m_eventBus.Unsubscribe(EventType::CoinGained, m_coinGainedListenerId);
    m_eventBus.Unsubscribe(EventType::CoinSpent, m_coinSpentListenerId);
    m_eventBus.Unsubscribe(EventType::ItemCrafted, m_itemCraftedListenerId);
    m_eventBus.Unsubscribe(EventType::PotionUsed, m_potionUsedListenerId);
    m_eventBus.Unsubscribe(EventType::DungeonAttemptUsed, m_dungeonAttemptListenerId);
}

// ==============================================================================
// Per-frame
// ==============================================================================

void QuestSystem::Update()
{
    // Cheap timestamp comparison — no heavy work unless a reset is due
    CheckResets();
}

// ==============================================================================
// Quest registration
// ==============================================================================

void QuestSystem::RegisterDailyQuests()
{
    m_dailyQuests.clear();

    // Kill 20 enemies
    m_dailyQuests.push_back({3001, "Monster Hunter",
                             "Kill 20 enemies today.",
                             QuestType::KillEnemies, QuestPeriod::Daily,
                             20, 0, 100});

    // Earn 200 coins
    m_dailyQuests.push_back({3002, "Fortune Seeker",
                             "Earn 200 coins today.",
                             QuestType::EarnCoins, QuestPeriod::Daily,
                             200, 0, 50});

    // Craft 1 item
    m_dailyQuests.push_back({3003, "Crafting Novice",
                             "Craft 1 item today.",
                             QuestType::CraftItems, QuestPeriod::Daily,
                             1, 0, 75});
}

void QuestSystem::RegisterWeeklyQuests()
{
    m_weeklyQuests.clear();

    // Kill 3 bosses this week
    m_weeklyQuests.push_back({4001, "Boss Slayer",
                              "Defeat 3 bosses this week.",
                              QuestType::KillBosses, QuestPeriod::Weekly,
                              3, 0, 500});

    // Kill 100 enemies this week
    m_weeklyQuests.push_back({4002, "Unstoppable",
                              "Kill 100 enemies this week.",
                              QuestType::KillEnemies, QuestPeriod::Weekly,
                              100, 0, 300});

    // Craft 5 items this week
    m_weeklyQuests.push_back({4003, "Master Crafter",
                              "Craft 5 items this week.",
                              QuestType::CraftItems, QuestPeriod::Weekly,
                              5, 0, 400});

    // Complete 5 stages
    m_weeklyQuests.push_back({4004, "Stage Conqueror",
                              "Complete 5 stages this week.",
                              QuestType::CompleteStages, QuestPeriod::Weekly,
                              5, 0, 600});
}

// ==============================================================================
// Progress advancement
// ==============================================================================

void QuestSystem::AdvanceQuests(QuestType type, int amount)
{
    // Check both daily and weekly quest lists
    auto advanceList = [&](std::vector<Quest> &quests)
    {
        for (auto &quest : quests)
        {
            if (quest.type != type)
                continue;
            if (quest.isCompleted)
                continue;

            quest.currentCount += amount;

            m_eventBus.Fire(EventType::QuestProgressUpdated,
                            QuestProgressUpdatedEvent{
                                quest.id,           // questId
                                quest.currentCount, // current
                                quest.goal          // goal
                            });

            if (quest.currentCount >= quest.goal)
            {
                quest.currentCount = quest.goal; // Clamp — don't go over
                quest.isCompleted = true;

                m_eventBus.Fire(EventType::QuestCompleted, QuestCompletedEvent{
                                                               quest.id,  // questId
                                                               quest.name // questName
                                                           });

                std::cout << "[QuestSystem] Quest completed: " << quest.name << "\n";
            }
        }
    };

    advanceList(m_dailyQuests);
    advanceList(m_weeklyQuests);
}

// ==============================================================================
// Reward collection
// ==============================================================================

int QuestSystem::ClaimReward(uint32_t questId)
{
    Quest *quest = FindQuest(questId);
    if (!quest)
        return 0;
    if (!quest->isCompleted)
        return 0;
    if (quest->isRewarded)
        return 0;

    quest->isRewarded = true;

    m_eventBus.Fire(EventType::QuestRewardClaimed, QuestRewardClaimedEvent{
                                                       quest->id,        // questId
                                                       quest->coinReward // coinsRewarded
                                                   });

    std::cout << "[QuestSystem] Reward claimed for: " << quest->name
              << " (+" << quest->coinReward << " coins)\n";

    return quest->coinReward;
}

// ==============================================================================
// Dungeon attempts
// ==============================================================================

int QuestSystem::GetDungeonAttemptsRemaining(int dungeonId) const
{
    auto it = m_dungeonAttemptsUsed.find(dungeonId);
    int used = (it != m_dungeonAttemptsUsed.end()) ? it->second : 0;
    return std::max(0, DUNGEON_MAX_ATTEMPTS - used);
}

bool QuestSystem::UseAttempt(int dungeonId)
{
    if (GetDungeonAttemptsRemaining(dungeonId) <= 0)
        return false;
    m_dungeonAttemptsUsed[dungeonId]++;

    m_eventBus.Fire(EventType::DungeonAttemptUsed, DungeonAttemptUsedEvent{
                                                       dungeonId,
                                                       GetDungeonAttemptsRemaining(dungeonId)});

    return true;
}

void QuestSystem::ResetDungeonAttempts()
{
    m_dungeonAttemptsUsed.clear();
    std::cout << "[QuestSystem] Dungeon attempts reset.\n";
}

void QuestSystem::CheckResets()
{
    if (IsNewDay())
        ResetDailyQuests();
    if (IsNewWeek())
        ResetWeeklyQuests();
}

void QuestSystem::ResetDailyQuests()
{
    m_lastDailyReset = std::time(nullptr);
    RegisterDailyQuests(); // Re-registers with fresh progress
    ResetDungeonAttempts();
    std::cout << "[QuestSystem] Daily quests reset.\n";
}

void QuestSystem::ResetWeeklyQuests()
{
    m_lastWeeklyReset = std::time(nullptr);
    RegisterWeeklyQuests();
    std::cout << "[QuestSystem] Weekly quests reset.\n";
}

bool QuestSystem::IsNewDay() const
{
    // Compare calendar dates of last reset vs now
    time_t now = std::time(nullptr);
    struct tm lastTm = *std::localtime(&m_lastDailyReset);
    struct tm nowTm = *std::localtime(&now);

    return nowTm.tm_yday != lastTm.tm_yday ||
           nowTm.tm_year != lastTm.tm_year;
}

bool QuestSystem::IsNewWeek() const
{
    time_t now = std::time(nullptr);
    struct tm lastTm = *std::localtime(&m_lastWeeklyReset);
    struct tm nowTm = *std::localtime(&now);

    // Week number differs or year differs
    return nowTm.tm_yday / 7 != lastTm.tm_yday / 7 ||
           nowTm.tm_year != lastTm.tm_year;
}

// ==============================================================================
// Lookup
// ==============================================================================

const Quest *QuestSystem::GetQuest(uint32_t questId) const
{
    return const_cast<QuestSystem *>(this)->FindQuest(questId);
}

Quest *QuestSystem::FindQuest(uint32_t questId)
{
    for (auto &q : m_dailyQuests)
        if (q.id == questId)
            return &q;
    for (auto &q : m_weeklyQuests)
        if (q.id == questId)
            return &q;
    return nullptr;
}