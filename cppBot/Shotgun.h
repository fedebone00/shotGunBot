#pragma once

#include <iostream>
#include <tgbot/tgbot.h>
using namespace std;
using namespace TgBot;

typedef struct Shotgun{
    long chatId;
    int creatorId, messageId;
    string messageText;
    InlineKeyboardMarkup::Ptr keyboard;
    Shotgun(long _chatId, int _creatorId);
    bool equals(Shotgun s) const;
    bool equals(Shotgun* sPtr) const;
} Shotgun;