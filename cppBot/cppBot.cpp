// cppBot.cpp: definisce il punto di ingresso dell'applicazione.
//

#include "cppBot.h"

using namespace std;
using namespace TgBot;

int answeringAt, answeringMessageId, BOT_ID;

int main(){
	char* key = getenv("SHOTGUNBOTKEY");
	const string token(key);
	vector<myUser> users;
	loadUsersFromFile(&users);
	vector<Shotgun> shotguns;
	Bot bot(token);
	BOT_ID = bot.getApi().getMe()->id;

	bot.getEvents().onCommand("start", [&users, &bot](Message::Ptr message) {
		try{handleStartCommand(&users, &bot, message);}
		catch(exception& e){
				cerr << e.what() << endl;
			}
		});
	bot.getEvents().onCommand("feedback", [&users, &bot](Message::Ptr message) {
		try{handleFeedbackCommand(&users, &bot, message);}
		catch(exception& e){cout << e.what() << endl;}
		});
	bot.getEvents().onCommand("cancel", [&users, &bot](Message::Ptr message) {
		try{handleCancelCommand(&users, &bot, message);}
		catch(exception& e){cerr << e.what() << endl;}
		});
	bot.getEvents().onCommand("create", [&shotguns, &users, &bot](Message::Ptr message) {
		try{handleCreateCommand(&shotguns, &users, &bot, message);}
		catch(exception& e){cerr << e.what() << endl;}
		});
	bot.getEvents().onCommand("reset", [&shotguns, &bot](Message::Ptr message) {
		try{handleResetCommand(&shotguns, &bot, message);}
		catch(exception& e){cerr << e.what() << endl;}
		});
	bot.getEvents().onNonCommandMessage([&users, &bot](Message::Ptr message) {
		try{handleNonCommand(&users, &bot, message);}
		catch(exception& e){cerr << e.what() << endl;}
		});
	bot.getEvents().onCallbackQuery([&shotguns, &users, &bot](CallbackQuery::Ptr callback) {
		try{handleCallbackQuery(&shotguns, &users, &bot, callback);}
		catch(exception& e){cerr << e.what() << endl;}
		});
		
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

void addUserToFile(myUser* u){
	ofstream file;
	
	char cwd[50];
	getcwd(cwd, sizeof(char)*50);

	file.open(string(cwd) + "/cppBot/users.txt", ofstream::app);
	if(file.fail()){
		cerr << "Errore nell'apertura del file in scrittura" << endl;
		return;
	}
	file << u->chatId << " " << u->id << endl;
	cout << "new user saved" << endl;
	file.close();
}

int findShotgun(vector<Shotgun>* shotguns, Shotgun* _shotgun){
	for (int i = 0; i < shotguns->size(); i++)
	{
		if(shotguns->at(i).chatId == _shotgun->chatId)
			return i;
	}
	return -1;
}

int getOrInsertUser(vector<myUser>* users, myUser* _user){
	int i;
	for (i = 0; i < users->size(); i++)
	{
		if(users->at(i).equals(_user))
			return i;
	}
	users->push_back(*_user);
	addUserToFile(_user);
	return i;
}

void handleStartCommand(vector<myUser>* users, Bot* bot, Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	myUser* tmp = new myUser(message->chat->id, message->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
	myUser* user = &(users->at(userIndex));
	bot->getApi().sendMessage(user->chatId, "Ciao, sono un bot demmerda scritto in c++, porta pazienza"
		" e segnala i bug tramite il comando /feedback\n\n"
		"Digita /create seguito da una serie di numeri delimitato da spazio"
		" che rappresentano il numero di sedili per fila.\n"
		"Ad esempio una multipla sarà\n/create 3 3");
}

void handleFeedbackCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	myUser* tmp = new myUser(message->chat->id, message->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
	myUser* user = &(users->at(userIndex));
	user->state = FEEDBACK;
	bot->getApi().sendMessage(user->chatId, "Invia il tuo feedback o digita /cancel per terminare");
}

void handleCancelCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	myUser* tmp = new myUser(message->chat->id, message->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
	myUser* user = &(users->at(userIndex));
	if(user->state != NORMAL){
		user->state = NORMAL;
		bot->getApi().sendMessage(user->chatId, "Operazione annullata");
	} else {
		bot->getApi().sendMessage(user->chatId, "Nessuna operazione da annullare");
	}
}

void handleCreateCommand(vector<Shotgun>* shotguns, vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	myUser* tmp = new myUser(message->chat->id, message->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
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
	Shotgun* s = new Shotgun(user->chatId, user->id);
	int shotgunIndex = findShotgun(shotguns, s);
	if(shotgunIndex==-1){
		shotguns->push_back(*s);
		shotgunIndex = shotguns->size()-1;
		inserito = true;
	}
	delete s;
	Shotgun* shotgun = &(shotguns->at(shotgunIndex));

	if(inserito){
		shotgun->keyboard = InlineKeyboardMarkup::Ptr(new InlineKeyboardMarkup);
		string info = "";
		for(int i=0; i<options.size(); i++){
			int sedili;
			try{
				sedili = stoi(options[i]);
			} catch (exception& ex) {
				info += (info=="") ? "*Info: *"+options[i]+" " : options[i]+" "; 
				continue;
			}

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
		+ info + "\n"
		"Pronti... Ai vostri posti... *SHOTGUN*";

		Message::Ptr m = bot->getApi().sendMessage(user->chatId, mex, false, 0, shotgun->keyboard, "Markdown");
		shotgun->messageId = m->messageId;
		shotgun->messageText = mex;
		if(message->chat->type == Chat::Type::Group){
			bot->getApi().pinChatMessage(user->chatId, shotgun->messageId);
		}
	} else {
		bot->getApi().sendMessage(user->chatId, "Uno shotgun è già in corso, concludi quello e riprova");
	}
}

void handleResetCommand(vector<Shotgun>* shotguns, TgBot::Bot* bot, TgBot::Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	for(int i=0; i<shotguns->size(); i++){
		if(shotguns->at(i).chatId == message->chat->id){
			bot->getApi().deleteMessage(message->chat->id, shotguns->at(i).messageId);
			shotguns->erase(shotguns->begin()+i);
		}
	}
	bot->getApi().sendMessage(message->chat->id, "Reset avvenuto con successo, eliminate tutte le istanze shotgun aperte in questa chat");
}

void handleNonCommand(vector<myUser>* users, TgBot::Bot* bot, TgBot::Message::Ptr message){
	bot->getApi().sendChatAction(message->chat->id, "typing");
	myUser* tmp = new myUser(message->chat->id, message->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
	myUser* user = &(users->at(userIndex));

	InlineKeyboardMarkup::Ptr kb(new InlineKeyboardMarkup());
	vector<InlineKeyboardButton::Ptr> row0;
	InlineKeyboardButton::Ptr btn(new InlineKeyboardButton());

	string m;
	switch (user->state)
	{
	case NORMAL:
		if(message->chat->type==Chat::Type::Private) bot->getApi().sendMessage(user->chatId, "Non è il momento di scrivere cose a caso");
		break;

	case FEEDBACK:
		m = "Nuovo feedback ricevuto!\n" + message->text;
		btn->text = "Rispondi a @" + message->from->username;
		btn->callbackData = "answer;" + to_string(message->messageId) + ";" + to_string(user->chatId);
		row0.push_back(btn);
		kb->inlineKeyboard.push_back(row0);
		bot->getApi().sendMessage(user->chatId, "Feedback inviato con successo!");
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
	myUser* tmp = new myUser(callback->message->chat->id, callback->from->id);
	int userIndex = getOrInsertUser(users, tmp);
	delete tmp;
	myUser* user = &(users->at(userIndex));
	vector<string> options = StringTools::split(callback->data, ';');

	if(options[0] == "answer"){
		user->state = ANSWER;
		answeringMessageId = stoi(options[1]);
		answeringAt = stoi(options[2]);
		bot->getApi().sendMessage(DEV_ID, "Scrivi la risposta al feedback o digita /cancel");
	}
	else if(options[0] == "shotgun"){
		if(options[1] == "stop"){
			Shotgun* s = new Shotgun(stoi(options[2]), 0);
			Shotgun* shotgun = NULL;
			int shotgunIndex = findShotgun(shotguns, s);
			if(shotgunIndex!=-1){
				shotgun = &(shotguns->at(shotgunIndex));
			} else {
				bot->getApi().deleteMessage(callback->message->chat->id, callback->message->messageId);
				return;
			}
			delete s;

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
				bot->getApi().unpinChatMessage(user->chatId);

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

		bool inserito = false;
		Shotgun* s = new Shotgun(stoi(options[1]), user->id);
		int shotgunIndex = findShotgun(shotguns, s);
		if(shotgunIndex==-1){
			shotguns->push_back(*s);
			shotgunIndex = shotguns->size()-1;
			inserito = true;
		}
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
			shotgun->keyboard->inlineKeyboard[i][j]->callbackData = "shotgun;" + to_string(shotgun->chatId) + ";" + to_string(i) + ";" + to_string(j);
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
}
