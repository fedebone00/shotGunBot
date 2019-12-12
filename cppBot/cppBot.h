// cppBot.h: file di inclusione per file di inclusione di sistema standard,
// o file di inclusione specifici del progetto.

#pragma once

#include "User.h"
#include <iostream>
#include <tgbot/tgbot.h>
#include <vector>

// TODO: Fare riferimento qui alle intestazioni aggiuntive richieste dal programma.

char* dev_id = getenv("TELEGRAMDEVID");
const int DEV_ID = dev_id!=NULL ? std::stoi(dev_id) : 0;
void MergeSort(vector<myUser> *users, int p, int r);
void Merge(vector<myUser> *users, int p, int q, int r);
void loadUsersFromFile(vector<myUser> *v);
void addUserToFile(myUser* u);
int findShotgun(vector<Shotgun>* shotguns, Shotgun* _shotgun);
void handleStartCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message);
void handleFeedbackCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message);
void handleCancelCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message);
void handleCreateCommand(vector<Shotgun>* shotguns, vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message);
void handleNonCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message);
void handleCallbackQuery(vector<Shotgun>* shotguns, vector<myUser>* users, TgBot::Bot* bot, TgBot::CallbackQuery::Ptr callback);
