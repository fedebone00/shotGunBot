#pragma once

#include <iostream>
#include <tgbot/tgbot.h>
using namespace std;
using namespace TgBot;

typedef enum {NORMAL, FEEDBACK, ANSWER} State;

typedef struct myUser{
    long chatId;
    int id;
    State state;
    myUser();
    myUser(long _chatId, int _id, State _state = NORMAL);
    bool equals(myUser u) const;
    bool equals(myUser* uPtr) const;
} myUser;