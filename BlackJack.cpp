#include <iostream> 
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <list>
#include <string>
#include <conio.h>
#include <sstream>

using namespace std;

queue<int> m_Deck;
list<int> m_PlayerCards;
list<int> m_DealerCards;
list<int> m_SplitCards;

int m_iPlayerMoney;
int m_iDealerMoney;
int m_iBet;
int m_iSplitBet;
bool m_bInsurance;

enum target : int
{
	Player = 0,
	Dealer = 1,
	Split = 2,
};

enum gameoverRule : int
{
	NoRule = 0,
	MoneyToZero = 1,
	DeckLessThenTenCard = 2,
	Endless = 3,
};
gameoverRule m_Rule;

enum gameResult : int
{
	NoResult = 0,
	PlayerWin = 1,
	DealerWin = 2,
	Draw = 3,
};
gameResult m_Result;
gameResult m_SplitResult;

void StartSetting(int &iCardPack);
void ShuffleDeck(int iPackNum);
void BetStep();
bool DoubleStep(bool bMainHand);
void GiveCard(target t);
void SplitStep();
bool SurrenderStep();
void InsuranceStep();
void DisplayMoneyAndDeck();
void DisplayMatch(bool hideHole);
bool PlayerMove(bool bDouble, target t);
string CardSymbol(int number);
int CardValue(target t);
void FinalMatch();
void BetCalculation();
bool EndGameCheck();

int main()
{
	bool bReset = true;
	int iCardPack = 0;
	
	//Instruction Step
	cout << "This is a game called BlackJack" << endl;
	cout << "Your goal is getting points as close to 21 as possible" << endl;
	cout << "If you go over 21, you are busted, on the other word, you lose" << endl;
	cout << "If both player and dealer didn't busted, then who has higher points win" << endl;
	cout << "2~10 has same point as their number" << endl;
	cout << "A can treat as 1 or 11" << endl;
	cout << "K Q J worth 10 points" << endl;
	cout << "If you get 21 points by just 2 cards (A and another 10 points card) it's called BlackJack" << endl;
	cout << "Which beat other 21 points combination" << endl;
	cout << "If you finished reading and want to start" << endl;
	cout << "Just press any key to continue" << endl;
	getch();
	
	//While loop to keep the game going
	while(true)
	{
		//Initial some value
		m_iBet = 0;
		m_iSplitBet = 0;
		m_Result = NoResult;
		m_SplitResult = NoResult;
		m_bInsurance = false;
		
		//Action take for all new round
		if(bReset)
		{
			if (system("CLS")) system("clear");
			StartSetting(iCardPack);
			bReset = false;
		}
		
		if (system("CLS")) system("clear");
		//Shuffle Deck when the card are running low
		if(m_Deck.size() <= 10)
			ShuffleDeck(iCardPack);
		
		//Clear cards
		m_PlayerCards.clear();
		m_DealerCards.clear();
		m_SplitCards.clear();
		BetStep();

		//Draw 2 cards at start
		for(int i = 0; i < 2; i++)
		{
			GiveCard(Player);
			GiveCard(Dealer);
		}
		DisplayMatch(true);
		
		//SurrenderStep
		if(SurrenderStep())
			continue;
		
		//InsuranceStep happened when dealer's showing card is A
		if(m_DealerCards.front() == 1)
			InsuranceStep();
		
		//Split Step happend when you have two cards with same value
		int firstValue = m_PlayerCards.front();
		if(firstValue > 10)
			firstValue = 10;
		
		int nextValue = m_PlayerCards.back();
		if(nextValue > 10)
			nextValue = 10;
		
		if(firstValue == nextValue)
			SplitStep();
		
		//Double up step
		bool bDouble = DoubleStep(true);
		bool bBust = PlayerMove(bDouble, Player);
		
		if(bBust)
			m_Result = DealerWin;
		
		cout << "Press any key to continue" << endl;
		DisplayMatch(false);
		
		//Double up for split hand, skip if you don't have split
		if(m_SplitCards.size() > 0)
		{
			bDouble = DoubleStep(false);
			bBust = PlayerMove(bDouble, Split);
			
			if(bBust)
				m_SplitResult = DealerWin;
			
			cout << "Press any key to continue" << endl;
			DisplayMatch(false);
		}
		
		//Dealer's turn. According to rule, dealer can't split or double up, dealer have to draw until he has 17 points or higher and he must stop drawing after that
		//Skip if player already busted
		if(m_Result == NoResult || (m_SplitCards.size() > 0 && m_SplitResult == NoResult))
		{
			while(CardValue(Dealer) <= 17)
			{
				cout << "Press any key to continue" << endl;
				getch();
				GiveCard(Dealer);
				DisplayMatch(false);
				
				if(m_Deck.size() <= 0)
					break;
			}
		}
		
		if(CardValue(Dealer) > 21)
		{
			if(m_Result == NoResult)
				m_Result = PlayerWin;
			
			if(m_SplitResult == NoResult && m_SplitCards.size() > 0)
				m_SplitResult = PlayerWin;
		}
		
		//End and calculate step
		FinalMatch();
		BetCalculation();
		bReset = EndGameCheck();
		
		if(bReset)
		{
			cout << "Game over, Press any key for a new game" << endl;
			getch();
		}
		else
		{
			cout << "Press any key for next round" << endl;
			getch();
		}
	}
	
	return 0;
}

void StartSetting(int &iCardPack)
{
	//Some rule setting
	bool bSetting = true;
	while(bSetting)
	{
		iCardPack = 0;
		m_iPlayerMoney = 0;
		string sMoneyInput;
		
		//starting fund
		cout << "Please type in how much money you want at start" << endl;
		getline(cin, sMoneyInput);
		while(true)
		{
			stringstream tempStream(sMoneyInput);
			if(tempStream >> m_iPlayerMoney && m_iPlayerMoney > 0)
				break;
			
			cout << "Invalid entry, please try again" << endl;
			getline(cin, sMoneyInput);
		}
		
		//end game condition
		cout << "Please select when to end the game" << endl;
		cout << "1. When either dealer or player money go blow 0" << endl;
		cout << "2. When deck's card is less then 10" << endl;
		cout << "3. It will never end" << endl;
		
		m_Rule = NoRule;
		while(m_Rule == NoRule)
		{
			cout << "Type the number you want to chose" << endl;
			int iRuleSelect;
			string sRuleSelect;
			getline(cin, sRuleSelect);
			stringstream tempStream(sRuleSelect);
			if(!(tempStream >> iRuleSelect))
			{
				cout << "Please type the number you want to choose" << endl;
				continue;
			}
			
			switch(iRuleSelect)
			{
				case 1:
					m_Rule = MoneyToZero;
					break;
					
				case 2:
					m_Rule = DeckLessThenTenCard;
					break;
					
				case 3:
					m_Rule = Endless;
					break;
					
				default:
					cout << "Invalid entry, please try again" << endl;
					continue;
			}
		}
		
		string sPackInput;
		
		//card pack use, due to only 1 vs 1, so I set the pack limit to 4
		cout << "Please type in how much pack of card you want to use (between 1~4)" << endl;
		getline(cin, sPackInput);
		while(true)
		{
			stringstream tempStream(sPackInput);
			if(tempStream >> iCardPack && iCardPack > 0 && iCardPack <= 4)
				break;
			
			cout << "Invalid entry, please try again" << endl;
			getline(cin, sPackInput);
		}
		
		//Final check
		cout << "Please check is this what you want to start the game?" << endl;
		cout << "Starting money: " <<  m_iPlayerMoney << endl;
		cout << "Pack use: " <<  iCardPack << endl;
		cout << "End game condition: ";
		switch(m_Rule)
		{
			case MoneyToZero:
				cout << "When either dealer or player's money go below zero" << endl;
				break;
				
			case DeckLessThenTenCard:
				cout << "When deck cards is fewer than 10" << endl;
				break;
				
			case Endless:
				cout << "The game will never end" << endl;
				break;
				
			default:
				cout << "Data error, start from begining again..." << endl;
				continue;
		}
		cout << "Press 1 to start the game or press 2 to reset from begining" << endl;
		
		while(true)
		{
			char cFinalCheck = getch();
			switch(cFinalCheck)
			{
				case '1':
					bSetting = false;
					m_iDealerMoney = m_iPlayerMoney;
					cout << "OK, please press any key to start the game" << endl;
					getch();
					break;
					
				case '2':
					cout << "OK, we will start from begining again" << endl;
					break;
					
				default:
					cout << "Invalid entry, please try again" << endl;
					continue;
			}
			break;
		}
	}
	
	if (system("CLS")) system("clear");
	ShuffleDeck(iCardPack);
}

void ShuffleDeck(int iPackNum)
{
	//Set all card
	int iCardNum = iPackNum * 52;
	int allCard[iCardNum];
	for(int i = 0; i < (4 * iPackNum); i++)
	{
		for(int j = 0; j < 13; j++)
		{
			allCard[13 * i + j] = j + 1;
		}
	}
	
	//initialize random seed
	srand (time(NULL));
	
	//Shuffle
	for(int i = 0; i < iCardNum; i++)
	{
		int temp = rand() % (iCardNum - 1);
		int x = allCard[i];
		allCard[i] = allCard[temp];
		allCard[temp] = x;
	}
	
	queue<int> newDeck;
	for(int i = 0; i < iCardNum; i++)
		newDeck.push(allCard[i]);
	
	swap(m_Deck, newDeck);
	cout << "The Deck is shuffled" << endl;
	cout << "Deck : " << m_Deck.size() << endl;
	cout << "Press any key to start the round" << endl;
	getch();
}

void DisplayMoneyAndDeck()
{
	cout << "Deck : " << m_Deck.size() << endl;
	cout << "------------------------------------------------------------------" << endl;
	cout << "Player Money: " << m_iPlayerMoney << endl;
	cout << "Dealer Money: " << m_iDealerMoney << endl;
	cout << "Main hand's Bet: " << m_iBet << endl;
	if(m_iSplitBet > 0)
		cout << "Split hand's Bet: " << m_iSplitBet << endl;
	
	if(m_bInsurance)
		cout << "<<Insurance>>" << endl;
	
	cout << "------------------------------------------------------------------" << endl;
}

void BetStep()
{
	if (system("CLS")) system("clear");
	DisplayMoneyAndDeck();
	cout << "Please input the bet" << endl;
	
	string sBetInput;
	int iBetValue;
	getline(cin, sBetInput);
	while(true)
	{
		iBetValue = 0;
		stringstream tempStream(sBetInput);
		if(tempStream >> iBetValue && iBetValue > 0)
		{
			if(iBetValue > m_iPlayerMoney && m_Rule == MoneyToZero)
			{
				cout << "You Don't have enough money, please try again." << endl;
			}
			else
				break;
		}
		else
			cout << "Invalid entry, please try again" << endl;
		
		getline(cin, sBetInput);
	}
	
	m_iBet = iBetValue;
	
	cout << "You put your bet in, press any key to continue" << endl;
}

bool DoubleStep(bool bMainHand)
{
	char cDoubleInput;
	if(bMainHand)
		cout << "Do you want to double your bet for main hand? If you decide to your main hand will get another card and can't get any card for the round" << endl;
	else
		cout << "Do you want to double your bet for split hand? If you decide to your split hand will get another card and can't get any card for the round" << endl;
	
	cout << "1 for yes and 2 for no" << endl;
	cDoubleInput = getch();
	while(true)
	{
		switch(cDoubleInput)
		{
			case '1':
				if(bMainHand)
					m_iBet *= 2;
				else
					m_iSplitBet *= 2;
				
				DisplayMatch(true);
				return true;
				
			case '2':
				return false;
				
			default:
				cout << "Invalid entry, please try again" << endl;
				break;
				
		}
		cDoubleInput = getch();
	}
}

void SplitStep()
{
	cout << "Do you want to split your hand? It will treat as 2 different set of card" << endl;
	cout << "1 for yes and 2 for no" << endl;
	string sSplitInput;
	int iSplitValue;
	getline(cin, sSplitInput);
	while(true)
	{
		iSplitValue = 0;
		stringstream tempStream(sSplitInput);
		if(tempStream >> iSplitValue)
		{
			bool bOK = true;
			switch(iSplitValue)
			{
				case 1:
					m_SplitCards.push_back(m_PlayerCards.front());
					m_PlayerCards.pop_front();
					GiveCard(Player);
					GiveCard(Split);
					m_iSplitBet = m_iBet;
					DisplayMatch(true);
					return;
					
				case 2:
					return;
					
				default:
					cout << "Invalid entry, please try again" << endl;
					bOK = false;
					break;
					
			}
			
			if(bOK)
				break;
		}
		else
			cout << "Invalid entry, please try again" << endl;
		
		getline(cin, sSplitInput);
	}
}

bool SurrenderStep()
{
	if(m_iPlayerMoney <= 1 && m_Rule == MoneyToZero)
	{
		cout << "You will lose if you surrender" << endl;
		cout << "So, skip the surrender step" << endl;
		return false;
	}
	
	cout << "Want to surrender now? You will only lose half of the bet if you surrender" << endl;
	cout << "1 for yes and 2 for no" << endl;
	
	string sSurrenderInput;
	int iSurrenderValue;
	getline(cin, sSurrenderInput);
	
	int surrenderLose = (m_iBet / 2) + (m_iBet % 2);
	
	while(true)
	{
		iSurrenderValue = 0;
		stringstream tempStream(sSurrenderInput);
		if(tempStream >> iSurrenderValue)
		{
			switch(iSurrenderValue)
			{
				case 1:
					m_iPlayerMoney -= surrenderLose;
					m_iDealerMoney += surrenderLose;
					DisplayMatch(true);
					cout << "You decide to surrender, you lose $" << surrenderLose << endl;
					cout << "Press any key to start next round" << endl;
					getch();
					return true;
					
				case 2:
					return false;
					
				default:
					cout << "Invalid entry, please try again" << endl;
					return false;
					
			}
		}
		else
			cout << "Invalid entry, please try again" << endl;
		
		getline(cin, sSurrenderInput);
	}
	return false;
}

void InsuranceStep()
{
	cout << "Dealer has an Ace on his showing card, do you want to give dealer half of the bet as the insurance?" << endl;
	cout << "If you buy the insurance, dealer will not win any money if dealer got BlackJack" << endl;
	cout << "If dealer doesn't get BlackJack, then nothing happened" << endl;
	cout << "So, want to buy insurance?" << endl;
	cout << "1 for yes and 2 for no" << endl;
	
	string sInsuranceInput;
	int iInsuranceValue;
	getline(cin, sInsuranceInput);
	
	int priceInsurance = (m_iBet / 2) + (m_iBet % 2);
	
	while(true)
	{
		iInsuranceValue = 0;
		stringstream tempStream(sInsuranceInput);
		if(tempStream >> iInsuranceValue)
		{
			bool bOK = true;
			switch(iInsuranceValue)
			{
				case 1:
					m_bInsurance = true;
					m_iPlayerMoney -= priceInsurance;
					m_iDealerMoney += priceInsurance;
					DisplayMatch(true);
					return;
					
				case 2:
					return;
					
				default:
					cout << "Invalid entry, please try again" << endl;
					bOK = false;
					break;
					
			}
			
			if(bOK)
				break;
		}
		else
			cout << "Invalid entry, please try again" << endl;
		
		getline(cin, sInsuranceInput);
	}
}

void GiveCard(target t)
{
	if(m_Deck.size() <= 0)
	{
		cout << "No more card in deck..." << endl;
		return;
	}
	
	switch(t)
	{
		case Player:
			m_PlayerCards.push_back(m_Deck.front());
			break;
			
		case Dealer:
			m_DealerCards.push_back(m_Deck.front());
			break;
			
		case Split:
			m_SplitCards.push_back(m_Deck.front());
			break;
	}
	
	m_Deck.pop();
}

void DisplayMatch(bool hideHole)
{
	if (system("CLS")) system("clear");
	DisplayMoneyAndDeck();
	
	list<int>::iterator it;
	
	//Accourding to rule, one of dealer's card remain face down until his turn and that card often called hole card
	if(hideHole)
	{
		cout << "Dealer :" << CardSymbol(m_DealerCards.front()) << " Hole"  << endl;
	}
	else
	{
		cout << "Dealer :";
		for(it = m_DealerCards.begin(); it != m_DealerCards.end(); it++)
			cout << CardSymbol(*it) << " ";
		
		cout << endl;
	}
	
	cout << "Player :";
	for(it = m_PlayerCards.begin(); it != m_PlayerCards.end(); it++)
		cout << CardSymbol(*it) << " ";
	
	cout << endl;
	
	//Only display splite if needed
	if(m_SplitCards.size() > 0)
	{
		cout << "Split :";
		for(it = m_SplitCards.begin(); it != m_SplitCards.end(); it++)
			cout << CardSymbol(*it) << " ";
	
		cout << endl;
	}
	cout << "------------------------------------------------------------------" << endl;
}

bool PlayerMove(bool bDouble, target t)
{
	char aInput;
	bool bEnd = false;
	int tempValue = 0;
	
	//If double down, player have to draw one more card and go stand
	if(bDouble)
	{
		GiveCard(t);
		tempValue = CardValue(t);
		DisplayMatch(true);
		return tempValue > 21;
	}
	
	//If not double down, go this way
	while(!bEnd)
	{
		cout << "Press 1 to stand or Press 2 to hit, no other input will be accept" << endl;
		aInput = getch();
		
		switch(aInput)
		{
			case '1':
				bEnd = true;
				break;
				
			case '2':
				GiveCard(t);
				tempValue = CardValue(t);
				DisplayMatch(true);
				if(tempValue > 21 || m_Deck.size() <= 0)
					bEnd = true;
				
				break;
					
				
			default:
			 cout << "unaccept input, please try again" << endl;
			 continue;
		}
	}
	
	return tempValue > 21;
}

string CardSymbol(int number)
{
	switch(number)
	{
		case 1:
			return "A";
		
		case 11:
			return "J";
		
		case 12:
			return "Q";
		
		case 13:
			return "K";
		
		default:
			return to_string(number);
	}
}

int CardValue(target t)
{
	list<int> theCards;
	list<int>::iterator it;
	switch(t)
	{
		case Player:
			theCards = m_PlayerCards;
			break;
			
		case Dealer:
			theCards = m_DealerCards;
			break;
			
		case Split:
			theCards = m_SplitCards;
			break;
			
		default:
			cout << "Unknown Player : " << t;
			return 0;
	}
	
	bool bAce = false;
	int iValue = 0;
	for(it = theCards.begin(); it != theCards.end(); it++)
	{
		int trueValue = *it;
		if(trueValue > 10)
			trueValue = 10;
		
		iValue += trueValue;
		if(trueValue == 1)
			bAce = true;
	}
	
	if(bAce && iValue <= 11)
		iValue += 10;
	
	return iValue;
}

void FinalMatch()
{
	int iDealerFinal;
	string sDealerFinal;
	bool bDealerBlackJack;
	
	iDealerFinal = CardValue(Dealer);
	sDealerFinal = to_string(iDealerFinal);
	bDealerBlackJack = (sDealerFinal == "21" && m_DealerCards.size() == 2);
	
	if(bDealerBlackJack)
		sDealerFinal = "BlackJack";
	
	//Main hand result check
	if(m_Result == PlayerWin)
		cout << "Dealer busted! ";
	else if(m_Result == DealerWin)
		cout << "Player's main hand busted! ";
	else
	{
		int iPlayerFinal = CardValue(Player);
		string sPlayerFinal = to_string(iPlayerFinal);
		bool bPlayerBlackJack = (sPlayerFinal == "21" && m_PlayerCards.size() == 2);
		
		if(bPlayerBlackJack)
			sPlayerFinal = "BlackJack";
		
		cout << sPlayerFinal << " vs " << sDealerFinal << ", ";
		if(iPlayerFinal > iDealerFinal)
			m_Result = PlayerWin;
		else if(iDealerFinal > iPlayerFinal)
			m_Result = DealerWin;
		else
		{
			if(bPlayerBlackJack && !bDealerBlackJack)
				m_Result = PlayerWin;
			else if(!bPlayerBlackJack && bDealerBlackJack)
				m_Result = DealerWin;
			else
				m_Result = Draw;
		}
	}
	
	switch(m_Result)
	{
		case PlayerWin:
			cout << "Player Win in main hand!" << endl;
			break;
			
		case DealerWin:
			cout << "Dealer Win in main hand!" << endl;
			break;
			
		case Draw:
			cout << "Draw in main hand!" << endl;
			break;
			
		default:
			cout << "Result Error in main hand!" << endl;
			break;
	}
	
	//Splite result check if needed
	if(m_SplitCards.size() > 0)
	{
		if(m_SplitResult == PlayerWin)
			cout << "Dealer busted! ";
		else if(m_SplitResult == DealerWin)
			cout << "Player's split hand busted! ";
		else
		{
			int iSplitFinal = CardValue(Split);
			string sSplitFinal = to_string(iSplitFinal);
			bool bSplitBlackJack = (sSplitFinal == "21" && m_SplitCards.size() == 2);
			
			if(bSplitBlackJack)
				sSplitFinal = "BlackJack";
			
			cout << sSplitFinal << " vs " << sDealerFinal << ", ";
			if(iSplitFinal > iDealerFinal)
				m_SplitResult = PlayerWin;
			else if(iDealerFinal > iSplitFinal)
				m_SplitResult = DealerWin;
			else
			{
				if(bSplitBlackJack && !bDealerBlackJack)
					m_SplitResult = PlayerWin;
				else if(!bSplitBlackJack && bDealerBlackJack)
					m_SplitResult = DealerWin;
				else
					m_SplitResult = Draw;
			}
		}
		
		switch(m_SplitResult)
		{
			case PlayerWin:
				cout << "Player Win in split hand!" << endl;
				break;
				
			case DealerWin:
				cout << "Dealer Win in split hand!" << endl;
				break;
				
			case Draw:
				cout << "Draw in split hand!" << endl;
				break;
				
			default:
				cout << "Result Error in split hand!" << endl;
				break;
		}
	}
}

void BetCalculation()
{
	//Bet calculation for main hand
	switch(m_Result)
	{
		case PlayerWin:
			cout << "Player get $" << m_iBet << " by winning the main hand" << endl;
			m_iPlayerMoney += m_iBet;
			m_iDealerMoney -= m_iBet;
			break;
			
		case DealerWin:
			if(m_bInsurance && CardValue(Dealer) == 21 && m_DealerCards.size() == 2)
			{
				cout << "Player doesn't lose any money due to insurance." << endl;
			}
			else
			{
				cout << "Player lose $" << m_iBet << " by losing the main hand" << endl;
				m_iDealerMoney += m_iBet;
				m_iPlayerMoney -= m_iBet;
			}
			break;
			
		case Draw:
			cout << "Main hand is a draw" << endl;
			break;
			
			
		default:
			cout << "Result Error!" << endl;
			break;
			
	}
	
	//Bet calculation for splite hand if needed
	switch(m_SplitResult)
	{
		case NoResult:
			//Do Nothing;
			break;
		
		case PlayerWin:
			cout << "Player get $" << m_iSplitBet << " by winning the split hand" << endl;
			m_iPlayerMoney += m_iSplitBet;
			m_iDealerMoney -= m_iSplitBet;
			break;
			
		case DealerWin:
			if(m_bInsurance && CardValue(Dealer) == 21 && m_DealerCards.size() == 2)
			{
				cout << "Player doesn't lose any money due to insurance." << endl;
			}
			else
			{
				cout << "Player lose $" << m_iSplitBet << " by losing the split hand" << endl;
				m_iDealerMoney += m_iSplitBet;
				m_iPlayerMoney -= m_iSplitBet;
			}
			break;
			
		case Draw:
			cout << "Split hand is a draw" << endl;
			break;
			
			
		default:
			cout << "Split Result Error!" << endl;
			break;
			
	}
	m_iBet = 0;
	m_iSplitBet = 0;
}

bool EndGameCheck()
{
	switch(m_Rule)
	{
		case MoneyToZero:
			if(m_iPlayerMoney <= 0 || m_iDealerMoney <= 0)
			{
				cout << "This round of game is over, press any key to show the final result" << endl;
				getch();
				if (system("CLS")) system("clear");
				DisplayMoneyAndDeck();
				
				if(m_iPlayerMoney <= 0)
					cout << "Player is out of money, player lose!" << endl;
				else
					cout << "Dealer is out of money, player win!" << endl;
				
				return true;
			}
			else
				return false;
		
		case DeckLessThenTenCard:
			if(m_Deck.size() <= 10)
			{
				cout << "This round of game is over, press any key to show the final result" << endl;
				getch();
				if (system("CLS")) system("clear");
				DisplayMoneyAndDeck();
				
				if(m_iPlayerMoney < m_iDealerMoney)
					cout << "Dealer has more money left, player lose!" << endl;
				else if(m_iDealerMoney < m_iPlayerMoney)
					cout << "Player has more money left, player win!" << endl;
				else
					cout << "Player and dealer has same amount of money, it's a draw!" << endl;
				
				return true;
			}
			else
				return false;
		
		case Endless:
			return false;
		
		default:
			cout << "Rule Error" << endl;
			return false;
	}
}


