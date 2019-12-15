#include "User.h"
using namespace std;

myUser::myUser(){}

myUser::myUser(long _chatId, int _id, State _state){
    chatId = _chatId;
    id = _id;
    state = _state;
}

bool myUser::equals(myUser u) const{
    return id == u.id && chatId == u.chatId;
}

bool myUser::equals(myUser* uPtr) const{
    return id == uPtr->id && chatId == uPtr->chatId;
}