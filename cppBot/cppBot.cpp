﻿// cppBot.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "cppBot.h"

using namespace std;
using namespace TgBot;

int answeringAt, answeringMessageId;

int main(){
	const string token(getenv("SHOTGUNBOTKEY"));
	vector<myUser> users;
	loadUsersFromFile(&users);
	vector<Shotgun> shotguns;
	Bot bot(token);

	bot.getEvents().onCommand("start", [&users, &bot](Message::Ptr message) {handleStartCommand(&users, &bot, message);});
	bot.getEvents().onCommand("feedback", [&users, &bot](Message::Ptr message) {handleFeedbackCommand(&users, &bot, message);});
	bot.getEvents().onCommand("cancel", [&users, &bot](Message::Ptr message) {handleCancelCommand(&users, &bot, message);});
	bot.getEvents().onCommand("create", [&shotguns, &users, &bot](Message::Ptr message) {handleCreateCommand(&shotguns, &users, &bot, message);});
	bot.getEvents().onNonCommandMessage([&users, &bot](Message::Ptr message) {handleNonCommand(&users, &bot, message);});
	bot.getEvents().onCallbackQuery([&shotguns, &users, &bot](CallbackQuery::Ptr callback) {handleCallbackQuery(&shotguns, &users, &bot, callback);});

	try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (exception& e) {
        printf("error: %s\n", e.what());
    }
	return 0;
}

void loadUsersFromFile(vector<myUser> *v){
	ifstream file;
	long _chatId;
	int _id;

	char cwd[50];
	getcwd(cwd, sizeof(char)*50);

	file.open(string(cwd) + "/cppBot/users.txt");
	if(file.fail()){
		cerr << "Errore nell'apertura del file in lettura" << endl;
		return;
	}
	while(file >> _chatId >> _id){
		v->push_back(myUser(_chatId, _id));
	}
	file.close();
	cout << "loaded " << v->size() << endl;
}

void addUserToFile(myUser *u){
	ofstream file;
	
	char cwd[50];
	getcwd(cwd, sizeof(char)*50);

	file.open(string(cwd) + "/cppBot/users.txt");
	if(file.fail()){
		cerr << "Errore nell'apertura del file in scrittura" << endl;
		return;
	}
	file << u->chatId << " " << u->id << endl;
	cout << "added" << endl;
	file.close();
}

int getOrInsertUser(vector<myUser>* users, myUser _user){
	if(users->size() == 0){
		users->insert(users->begin(), _user);
		addUserToFile(&_user);
		return 0;
	}
	int bottom = 0, top = users->size()-1, middle;
	
	while(top >= bottom){
		middle = (top+bottom)/2;

		if(_user.id < users->at(middle).id) top = middle-1;
		else if(_user.id > users->at(middle).id) bottom = middle+1;
		else if(_user.chatId == users->at(middle).chatId) return middle;
		else break;
	}

	addUserToFile(&_user);
	if(_user.id < users->at(middle).id){
		users->insert(users->begin()+middle-1, _user);
		return middle-1;
	} else {
		users->insert(users->begin()+middle+1, _user);
		return middle+1;
	}
}

int getOrInsertShotgun(vector<Shotgun>* shotguns, Shotgun _shotgun, bool* inserito){
	if(shotguns->size() == 0){
		*inserito = true;
		shotguns->insert(shotguns->begin(), _shotgun);
		return 0;
	}
	int bottom = 0, top = shotguns->size()-1, middle;
	
	while(top >= bottom){
		middle = (top+bottom)/2;

		if(_shotgun.chatId < shotguns->at(middle).chatId) top = middle-1;
		else if(_shotgun.chatId > shotguns->at(middle).chatId) bottom = middle+1;
		else{
			*inserito = false;
			return middle;
		}
	}

	*inserito = true;
	if(_shotgun.chatId < shotguns->at(middle).chatId){
		shotguns->insert(shotguns->begin()+middle-1, _shotgun);
		return middle-1;
	} else {
		shotguns->insert(shotguns->begin()+middle+1, _shotgun);
		return middle+1;
	}
}

void handleStartCommand(vector<myUser>* users, Bot* bot, Message::Ptr message){
	int userIndex = getOrInsertUser(users, myUser(message->chat->id, message->from->id));
	myUser* user = &(users->at(userIndex));
	bot->getApi().sendMessage(user->chatId, "Ciao, sono un bot demmerda scritto in c++, porta pazienza"
		" e segnala i bug tramite il comando /feedback\n\n"
		"Digita /create seguito da una serie di numeri delimitato da spazio"
		" che rappresentano il numero di sedili per fila.\n"
		"Ad esempio una multipla sarà\n/create 3 3");
}

void handleFeedbackCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	int userIndex = getOrInsertUser(users, myUser(message->chat->id, message->from->id));
	myUser* user = &(users->at(userIndex));
	user->state = FEEDBACK;
	bot->getApi().sendMessage(user->chatId, "Invia il tuo feedback o digita /cancel per terminare");
}

void handleCancelCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	int userIndex = getOrInsertUser(users, myUser(message->chat->id, message->from->id));
	myUser* user = &(users->at(userIndex));
	user->state = NORMAL;
	bot->getApi().sendMessage(user->chatId, "Operazione annullata");
}

void handleCreateCommand(vector<Shotgun>* shotguns, vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	int userIndex = getOrInsertUser(users, myUser(message->chat->id, message->from->id));
	myUser* user = &(users->at(userIndex));
	
	vector<string> options = StringTools::split(message->text, ' ');
	options.erase(options.begin());

	if(options.size() == 0 || options[0] == "-h" || options[0] == "--help"){
		bot->getApi().sendMessage(user->chatId, 
		"Digita /create seguito da una serie di numeri delimitato da spazio"
		" che rappresentano il numero di sedili per fila.\n"
		"Ad esempio una multipla sarà\n/create 3 3");
		return;
	}

	bool inserito = false;
	int shotgunIndex = getOrInsertShotgun(shotguns, Shotgun(user->chatId, user->id), &inserito);
	Shotgun* shotgun = &(shotguns->at(shotgunIndex));

	if(inserito){
		shotgun->keyboard = InlineKeyboardMarkup::Ptr(new InlineKeyboardMarkup);

		for(int i=0; i<options.size(); i++){
			int sedili = stoi(options[i]);
			vector<InlineKeyboardButton::Ptr> row(sedili);
			row.clear();
			for(int j=0; j<sedili; j++){
				InlineKeyboardButton::Ptr btn(new InlineKeyboardButton);
				if(i == 0 && j==0) btn->text = "@" + message->from->username;
				else btn->text = "Libero";
				btn->callbackData = "shotgun;" + to_string(shotgun->chatId) + ";" + to_string(i) + ";" + to_string(j);
				row.push_back(btn);
			}
			shotgun->keyboard->inlineKeyboard.push_back(row);
		}
		
		vector<InlineKeyboardButton::Ptr> lRow;
		InlineKeyboardButton::Ptr btn(new InlineKeyboardButton);
		btn->text = "Concludi";
		btn->callbackData = "shotgun;stop;" + to_string(shotgun->chatId);
		lRow.push_back(btn);
		shotgun->keyboard->inlineKeyboard.push_back(lRow);

		string mex = "*Attenzione Attenzione!*\n"
		"Pare che @" + message->from->username + " voglia offrire un passaggio\n"
		"Pronti... Ai vostri posti... *SHOTGUN*";

		Message::Ptr m = bot->getApi().sendMessage(user->chatId, mex, false, 0, shotgun->keyboard, "Markdown");
		shotgun->messageId = m->messageId;
		shotgun->messageText = mex;
	} else {
		bot->getApi().sendMessage(user->chatId, "Uno shotgun è già in corso, concludi quello e riprova");
	}
}

void handleNonCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	int userIndex = getOrInsertUser(users, myUser(message->chat->id, message->from->id));
	
	myUser* user = &(users->at(userIndex));

	InlineKeyboardMarkup::Ptr kb(new InlineKeyboardMarkup());
	vector<InlineKeyboardButton::Ptr> row0;
	InlineKeyboardButton::Ptr btn(new InlineKeyboardButton());

	string m;
	switch (user->state)
	{
	case NORMAL:
		//bot->getApi().sendMessage(user->chatId, "Non è il momento di scrivere cose a caso");
		break;

	case FEEDBACK:
		m = "Nuovo feedback ricevuto!\n" + message->text;
		btn->text = "Rispondi a @" + message->from->username;
		btn->callbackData = "answer;" + to_string(message->messageId) + ";" + to_string(user->chatId);
		row0.push_back(btn);
		kb->inlineKeyboard.push_back(row0);
		bot->getApi().sendMessage(DEV_ID, m, false, 0, kb);
		break;
	
	case ANSWER:
		bot->getApi().sendMessage(answeringAt, message->text, false, answeringMessageId);
		bot->getApi().sendMessage(DEV_ID, "Risposta inviata con successo");
		break;

	default:
		break;
	}
}

void handleCallbackQuery(vector<Shotgun>* shotguns, vector<myUser>* users, TgBot::Bot* bot, TgBot::CallbackQuery::Ptr callback){
	bot->getApi().answerCallbackQuery(callback->id);
	int userIndex = getOrInsertUser(users, myUser(callback->message->chat->id, callback->from->id));
	myUser* user = &(users->at(userIndex));
	vector<string> options = StringTools::split(callback->data, ';');

	if(options[0] == "answer"){
		user->state = ANSWER;
		answeringMessageId = stoi(options[1]);
		answeringAt = stoi(options[2]);
		bot->getApi().sendMessage(DEV_ID, "Scrivi la risposta al feedback o digita /cancel");
	}
	else if(options[0] == "shotgun"){
		bool inserito;
		if(options[1] == "stop"){
			int shotgunIndex = getOrInsertShotgun(shotguns, Shotgun(stoi(options[2]), 0), &inserito);
			Shotgun* shotgun = &(shotguns->at(shotgunIndex));

			if(user->id == shotgun->creatorId){
				for(int n=0; n<shotgun->keyboard->inlineKeyboard.size(); n++){
					for(int m=0; m<shotgun->keyboard->inlineKeyboard[n].size(); m++){
						if(shotgun->keyboard->inlineKeyboard[n][m]->text == "@" + callback->from->username){
							shotgun->keyboard->inlineKeyboard[n][m]->callbackData = "skip";
						}
					}
				}
				shotgun->keyboard->inlineKeyboard.pop_back();
				
				//InlineKeyboardMarkup::Ptr kb(new InlineKeyboardMarkup());
				//*kb = *(shotgun->keyboard);

				bot->getApi().editMessageText(shotgun->messageText, shotgun->chatId, shotgun->messageId, "", "Markdown", false, shotgun->keyboard);
				shotguns->erase(shotguns->begin()+shotgunIndex);
				bot->getApi().sendMessage(user->chatId, "Shotgun terminato");
			} else {
				bot->getApi().sendMessage(user->chatId, "@" + callback->from->username + " vuole concludere lo shotgun, paura che ti rubino il posto?");
			}
			return;
		}
		if(StringTools::startsWith(options[1], "occupato")){
			if(stoi(options[2]) != user->id){
				string m = "@" + callback->from->username + " questo posto è già " + options[1];
				bot->getApi().sendMessage(user->chatId, m);
				return;
			}
		}

		int shotgunIndex = getOrInsertShotgun(shotguns, Shotgun(stoi(options[1]), user->id), &inserito);
		Shotgun* shotgun = &(shotguns->at(shotgunIndex));
		if(inserito){
			bot->getApi().sendMessage(user->chatId, "@" + callback->from->username + ", questo shotgun è già concluso!");
			shotguns->erase(shotguns->begin()+shotgunIndex);
			return;
		}

		int i=stoi(options[2]), 
		j=stoi(options[3]);

		if(i == 0 && j == 0 && user->id != shotgun->creatorId){
			string m = "@" + callback->from->username + " tenta il colpo di stato e vuole guidare";
			bot->getApi().sendMessage(user->chatId, m);
			return;
		}

		if(shotgun->keyboard->inlineKeyboard[i][j]->text == "@" + callback->from->username){
			shotgun->keyboard->inlineKeyboard[i][j]->text = "Libero";
			shotgun->keyboard->inlineKeyboard[i][j]->callbackData = "shotgun;occupato da @" + callback->from->username;
		} else {
			for(int n=0; n<shotgun->keyboard->inlineKeyboard.size(); n++){
				for(int m=0; m<shotgun->keyboard->inlineKeyboard[n].size(); m++){
					if(shotgun->keyboard->inlineKeyboard[n][m]->text == "@" + callback->from->username){
						shotgun->keyboard->inlineKeyboard[n][m]->text = "Libero";
						shotgun->keyboard->inlineKeyboard[n][m]->callbackData = "shotgun;" + to_string(shotgun->chatId) + ";" + to_string(n) + ";" + to_string(m);
					}
				}
			}
			shotgun->keyboard->inlineKeyboard[i][j]->text = "@" + callback->from->username;
			shotgun->keyboard->inlineKeyboard[i][j]->callbackData = "shotgun;" + to_string(shotgun->chatId) + ";" + to_string(i) + ";" + to_string(j);
		}

		bot->getApi().editMessageText(shotgun->messageText, shotgun->chatId, shotgun->messageId, "", "Markdown", false, shotgun->keyboard);
	}
	bot->getApi().answerCallbackQuery(callback->id);
}