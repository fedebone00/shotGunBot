#include "Shotgun.h"

Shotgun::Shotgun(long _chatId, int _creatorId){
    chatId = _chatId;
    creatorId = _creatorId;
}

bool Shotgun::equals(Shotgun s) const{
    return creatorId == s.creatorId && chatId == s.chatId;
}

bool Shotgun::equals(Shotgun* sPtr) const{
    return creatorId == sPtr->creatorId && chatId == sPtr->chatId;
}