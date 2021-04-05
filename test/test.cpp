#include <gtest/gtest.h>

#include "common/ttt.h"
#include "common/ttt_utils.h"

TEST(ttt, tttBeginRangeIsValid)
{
  const char* ttt =
    "..."
    "..."
    "..."
  ;
  EXPECT_EQ(static_cast<ttt_t>(EMPTY_TTT), from_string(ttt));
}

TEST(ttt, tttEndRangeIsValid)
{
  const char* ttt =
    "XXX"
    "XXX"
    "XXX"
  ;
  EXPECT_EQ(static_cast<ttt_t>(NUMBER_OF_TTT-1), from_string(ttt));
}

TEST(ttt, encodePlayerAsBool)
{
  EXPECT_NE(encodePlayerAsBool(Owner::Player0), encodePlayerAsBool(Owner::Player1));
}

TEST(ttt, other)
{
  EXPECT_EQ(OTHER(Owner::Player0), Owner::Player1);
  EXPECT_EQ(OTHER(Owner::Player1), Owner::Player0);
}

TEST(ttt, getSetTtt)
{
  for (int c=0; c<9; ++c)
  {
    for (auto owner : {Owner::None, Owner::Draw, Owner::Player0, Owner::Player1})
    {
      ttt_t ttt = EMPTY_TTT;
      // Testing the version with "c"
      set_ttt_int(ttt, c, owner);
      EXPECT_EQ(get_ttt_int(ttt, c), owner);
    }
  }

  for (int x=0; x<3; ++x)
    for (int y=0; y<3; ++y)
    {
      for (auto owner : {Owner::None, Owner::Draw, Owner::Player0, Owner::Player1})
      {
        ttt_t ttt = EMPTY_TTT;
        // Testing the version with "y, x"
        set_ttt_int(ttt, y, x, owner);
        EXPECT_EQ(get_ttt_int(ttt, y, x), owner);
      }
    }
}

TEST(ttt, nonesSimpleTest)
{
  EXPECT_EQ(nones(EMPTY_TTT), 9);
  EXPECT_EQ(nones(BIT0_IN_EACH), 0);
  EXPECT_EQ(nones(BIT1_IN_EACH), 0);
}

TEST(ttt, nonesEquivalenceTest)
{
  for (ttt_t ttt=0; ttt<NUMBER_OF_TTT; ++ttt)
  {
    // Simple way to count nones
    score_t simple_nones_count = 0;
    for (int c=0; c<9; ++c)
    {
      if (get_ttt_int(ttt, c) == Owner::None)
        simple_nones_count++;
    }

    EXPECT_EQ(nones(ttt), simple_nones_count);
  }
}

TEST(tttUtils, winSimpleTest)
{
  EXPECT_TRUE(win(BIT0_IN_EACH, Player0));
  EXPECT_FALSE(win(BIT0_IN_EACH, Player1));

  EXPECT_TRUE(win(BIT1_IN_EACH, Player1));
  EXPECT_FALSE(win(BIT1_IN_EACH, Player0));

  EXPECT_FALSE(win(EMPTY_TTT, Player0));
  EXPECT_FALSE(win(EMPTY_TTT, Player1));
}

TEST(tttUtils, winEquivalenceTest)
{
  for (ttt_t ttt=0; ttt<NUMBER_OF_TTT; ++ttt)
    for (auto player : {Owner::Player0, Owner::Player1})
    {
      bool simple_win = false;
      for (const auto& [c1, c2, c3] : ttt_possible_lines)
      {
        if (get_ttt_int(ttt, c1) == player && get_ttt_int(ttt, c2) == player && get_ttt_int(ttt, c3) == player)
        {
          simple_win = true;
          break;
        }
      }

      EXPECT_EQ(win(ttt, player), simple_win);
    }
}

TEST(tttUtils, winnableEquivalenceTest)
{
  for (ttt_t ttt=0; ttt<NUMBER_OF_TTT; ++ttt)
  {
    for (auto player : {Owner::Player0, Owner::Player1})
    {
      if (win(ttt, OTHER(player)))
        continue;

      bool simple_winnable = false;
      for (const auto& [c1, c2, c3] : ttt_possible_lines)
      {
        if ((get_ttt_int(ttt, c1) == player || get_ttt_int(ttt, c1) == Owner::None)
         && (get_ttt_int(ttt, c2) == player || get_ttt_int(ttt, c2) == Owner::None)
         && (get_ttt_int(ttt, c3) == player || get_ttt_int(ttt, c3) == Owner::None))
        {
          simple_winnable = true;
          break;
        }
      }

      EXPECT_EQ(winnable(ttt, player),  simple_winnable);
    }
  }
}
