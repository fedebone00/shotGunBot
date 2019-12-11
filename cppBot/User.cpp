#include "User.h"
using namespace std;

myUser::myUser(){}

myUser::myUser(long _chatId, int _id, State _state){
    chatId = _chatId;
    id = _id;
    state = _state;
}

bool myUser::equals(myUser u){
    return id == u.id;
}

Shotgun::Shotgun(long _chatId, int _creatorId){
    chatId = _chatId;
    creatorId = _creatorId;
}