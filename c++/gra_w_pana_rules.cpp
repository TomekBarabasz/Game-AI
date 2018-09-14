#include "pch.h"
#include "GameRules.h"
#include <random>
#include <sstream>
#include <iostream>

using std::wstring;
struct Card
{
	unsigned char value;
	unsigned char color;
	bool operator<(const Card& other) const { return this->value < other.value; }
	wstring toString() const
	{
		//static const wchar_t suits[]  = L"♥♠♣♦";
		static const wchar_t suits[]  = L"0123";
		static const wchar_t *values[] = { L"9",L"10",L"W",L"D",L"K",L"A" };
		std::wstringstream ss;
		ss << values[value-9] << suits[color];
		return ss.str();
	}
};

template <int SIZE>
struct CardSet
{
	static const int not_found = -1;
	Card cards[SIZE];
	int numCards;
	CardSet() : numCards(0) {}
	CardSet(std::initializer_list<Card> cards)
	{
		numCards = 0;
		for (auto & c : cards) append(c);
	}
	void append(const Card & c)
	{
		cards[numCards++] = c;
	}
	void remove(const Card & c)
	{
		auto *end = cards + numCards;
		auto it = std::lower_bound(cards, end, c);
		if (it == end) return;
		while (it->color != c.color) {
			++it;
		}
		while (it != end-1) {
			*it = *(it + 1);
		}
		--numCards;
	}
	void sort()
	{
		std::sort(cards, cards + numCards);
	}
	int find(const Card& c) const
	{
		auto *end = cards + numCards;
		auto it = std::lower_bound(cards, end, c);
		while (it != end && it->value == c.value)
		{
			if (it->color == c.color) {
				return int(it - cards);
			}
			++it;
		}
		return not_found;
	}
	wstring toString() const
	{
		if (0 == numCards) return wstring();
		std::wstringstream ss;
		for (int i=0; i<numCards; ++i)	{
			ss << cards[i].toString() << L" ";
		}
		return ss.str();
	}
	CardSet& createUsingAllCards()
	{
		int idx = 0;
		for (unsigned char v = 9; v <= 14; ++v)
			for (unsigned char c = 0; c < 4; ++c, ++idx)
			{
				cards[idx] = { v,c };
			}
		numCards = 24;
		return *this;
	}
	CardSet& shuffle()
	{
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, 23);

		for (auto i = 0; i < 20; ++i)
		{
			const int first = distribution(generator);
			const int second = distribution(generator);
			const Card tmp = cards[first];
			cards[first] = cards[second];
			cards[second] = tmp;
		}
		return *this;
	}
	CardSet<24> validCardsToPlay(const Card& tos) const
	{
		CardSet<24> vctp;
		for (auto i=0; i<numCards;++i)
		{
			const Card & c = cards[i];
			if (c.value >= tos.value) {
				vctp.append(c);
			}
		}
		vctp.sort();
		return vctp;
	}
	CardSet<6> validQuadsToPlay(const Card& tos) const
	{
		CardSet<6> vqtp;
		unsigned char last_value = 0;
		int suits_mask = 0;
		for (auto i = 0; i < numCards; ++i)
		{
			const Card & c = cards[i];
			if (c.value < tos.value) continue;
			if (c.value != last_value)
			{
				if (suits_mask == 0b1111) {
					vqtp.append( {last_value,0} );
				}
				last_value = c.value;
				suits_mask = 0;
			}
			suits_mask |= 1 << c.color;
		}
		if (suits_mask == 0b1111) {
			vqtp.append({ last_value,0 });
		}
		return vqtp;
	}
	void copy(const CardSet& set, int numCardsToCopy)
	{
		numCards = numCardsToCopy;
		for (auto i=0;i<numCardsToCopy;++i)
		{
			cards[i] = set.cards[i];
		}
	}
	
	const Card& first() const { return cards[0]; }
	const Card& last()  const { return cards[numCards - 1]; }
	Card* begin() { return cards; }
	const Card* begin() const { return cards; }
	Card* end() { return cards + numCards; }
	const Card* end() const { return cards + numCards; }
};
using Stack = CardSet<24>;
using Hand  = CardSet<24>;
struct GameState : IGameState
{
	const Card firstToPlay = { 9, 0 };
	Stack stack;
	Hand hand[4];
	int numPlayers;
	int currPlayer;
	static GameState* create() { return new GameState(); }
	void initialize(int numPlayers) override;
	MoveList getPlayerLegalMoves(int player) const override;
	IGameState* next(const MoveList& moves) const;
	void release() const override	{ delete this; }
	wstring toString() const override;
	bool isTerminal() const override
	{
		int numPlayersWithCards = 0;
		for (auto pn=0;pn<numPlayers;++pn) {
			if (hand[pn].numCards != 0) ++numPlayersWithCards;
		}
		return 1==numPlayersWithCards;
	}
	void score(int scoreArr[]) const override
	{
		if (isTerminal())
		{
			const int winPts = 100 / (numPlayers - 1);
			for (auto pn = 0; pn < numPlayers; ++pn) {
				scoreArr[pn] = hand[pn].numCards != 0 ? 0 : winPts;
			}
		}else
		{
			const int drawPts = 100 / numPlayers;
			for (auto pn = 0; pn < numPlayers; ++pn) {
				scoreArr[pn] = drawPts;
			}
		}
	}
};

struct BaseMove : IMove
{
	virtual GameState* apply(const GameState*cs, int player) const = 0;
};

struct Noop : BaseMove
{
	static Noop* create()
	{
		static Noop noop;
		return &noop;
	}
	void release() override { }
	wstring toString() const override { return L"noop"; }
	GameState* apply(const GameState* cs, int player) const override {
		return const_cast<GameState*>(cs);
	}
};

struct PlayCard : BaseMove, CardSet<4>
{
	static PlayCard* create( std::initializer_list<Card> cards ) {
		return new PlayCard(cards);
	}
	void release() override { delete this; }
	PlayCard(std::initializer_list<Card> cards) : CardSet(cards)
	{}
	wstring toString() const override
	{
		std::wstringstream ss;
		ss << L"Play cards : " << CardSet<4>::toString();
		return ss.str();
	}
	GameState* apply(const GameState* cs, int player) const override
	{
		auto *ns = GameState::create();
		ns->numPlayers = cs->numPlayers;
		ns->stack.copy(cs->stack, cs->stack.numCards);
		for (auto pn=0;pn<cs->numPlayers;++pn) {
			ns->hand[pn].copy(cs->hand[pn], cs->hand[pn].numCards);
		}
		for (auto i=0;i<numCards;++i) {
			auto & c = cards[i];
			ns->stack.append(c);
			ns->hand[player].remove(c);
		}
		return ns;
	}
};

template <int NUM_CARDS>
struct TakeCards : BaseMove
{
	static IMove* create() {
		static TakeCards<NUM_CARDS> tc;
		return &tc;
	}
	void release() override	{ }
	wstring toString() const override
	{
		std::wstringstream ss;
		ss << L"Take cards : " << NUM_CARDS;
		return ss.str();
	}
	GameState* apply(const GameState* cs, int player) const override
	{
		auto *ns = GameState::create();
		ns->numPlayers = cs->numPlayers;
		ns->stack.copy(cs->stack, cs->stack.numCards - NUM_CARDS);
		for(auto pn=0;pn<cs->numPlayers;++pn) {
			ns->hand[pn].copy(cs->hand[pn], cs->hand->numCards);
		}

		int last_idx = cs->stack.numCards - 1;
		for (auto i=0;i<NUM_CARDS;++i)
		{
			ns->hand[player].append(cs->stack.cards[last_idx--]);
		}
		ns->hand[player].sort();
		return ns;
	}
};

void GameState::initialize(int numPlayers)
{
	Hand allCards;
	allCards.createUsingAllCards().shuffle();
	numPlayers = numPlayers;
	const int NumCardsPerPlayer = 24 / numPlayers;
	for (int pn=0, cn=0; pn<numPlayers;++pn)
	{
		auto & phand = hand[pn];
		for (int ci=0;ci<NumCardsPerPlayer;++ci){
			phand.append( allCards.cards[cn++] );
		}
		phand.sort();
	}
	stack.numCards = 0;
	for (int pn=0;pn<numPlayers;++pn)
	{
		if (hand[pn].find(firstToPlay) != Hand::not_found) {
			currPlayer = pn;
			break;
		}
	}
}

MoveList GameState::getPlayerLegalMoves(int player) const
{
	if (player != currPlayer){
		return { Noop::create() };
	}

	MoveList moves;
	auto & phand = hand[player];

	if (0 == stack.numCards) 
	{
		//initial state - can play {9,0} or all four {9,_}
		if (phand.find(firstToPlay) != Hand::not_found) 
		{
			moves.push_back( PlayCard::create( { firstToPlay } ) );
			int suits_mask = 0;
			for (int i=0;i<4;++i) 
			{
				const Card & c = phand.cards[i];
				if (c.value == 9) {
					suits_mask |= 1 << c.color;
				}
				else break;
			}
			if (suits_mask == 0b1111) {
				moves.push_back(PlayCard::create({ {9,0}, {9,1}, {9,2}, {9,3} } ));
			}
		}
	}
	else
	{
		//PlayCards
		if (1==stack.numCards){
			int suits_mask = 0;
			for (int i=0;i<3;++i)
			{
				const Card & c = phand.cards[i];
				if (c.value == 9) {
					suits_mask |= 1 << c.color;
				}
				else break;
			}
			if (suits_mask == 0b1110) {
				moves.push_back(PlayCard::create({ {9,1}, {9,2}, {9,3} }));
			}
		}
		auto validCardsToPlay = phand.validCardsToPlay(stack.last());
		for (const auto c : validCardsToPlay)	{
			moves.push_back(PlayCard::create({ c }));
		}
		auto validQuadsToPlay = phand.validQuadsToPlay(stack.last());
		for (const auto c : validQuadsToPlay) {
			moves.push_back(PlayCard::create({ {c.value,0},{c.value,1},{c.value,2},{c.value,3} }));
		}
		//TakeCards
		if      (1 == stack.numCards){
			/*TakeCards not possible*/
		}
		else if (2 == stack.numCards){
			moves.push_back(TakeCards<1>::create());
		}
		else if (3 == stack.numCards){
			moves.push_back(TakeCards<2>::create());
		}else {
			moves.push_back(TakeCards<3>::create());
		}
	}

	return moves;
}

IGameState* GameState::next(const MoveList& moves) const
{
	const GameState *cs = this;
	GameState *ns = nullptr;
	for (auto pn=0; pn<moves.size(); ++pn)
	{
		auto mv = static_cast<BaseMove*>(moves[pn]);
		ns = mv->apply(cs, pn);
		if (ns != cs && cs != this) {
			delete cs;
		}
		cs = ns;
	}
	ns->currPlayer = (ns->currPlayer) % ns->numPlayers;
	return ns;
}

wstring GameState::toString() const
{
	std::wstringstream ss;
	ss << L"Stack : " << stack.toString() << std::endl;
	for (int i=0;i<numPlayers;++i){
		ss << L"Player " << i << L" hand : " << hand[i].toString() << std::endl;
	}
	ss << L"Current player : " << currPlayer;
	return ss.str();
}

IGameState* createGraWPanaGameState()
{
	return GameState::create();
}

void testGraWPanaRules()
{
	Hand h({ {9,0},{9,1},{9,2},{9,3},{10,0},{10,1},{11,0},{11,1},{11,2},{11,3}});
	auto vqtp = h.validQuadsToPlay({ 9,0 });
	std::wcout << vqtp.toString() << std::endl;
}