#include "pch.h"
#include "GameRules.h"
#include <random>
#include <sstream>
#include <iostream>
#include <functional>
#include <boost/thread/mutex.hpp>

using std::wstring;

template <typename T>
struct ObjectPool
{
	std::vector<T*>  objects;
	int numAliveObjects = 0;

	template <typename...Args>
	T* alloc(Args...args)
	{
		++numAliveObjects;
		if (!objects.empty())
		{
			T* obj = objects.back(); objects.pop_back();
			return obj;
		}
		return new T(std::forward<Args>(args)...);
	}
	void free(T* obj)
	{
		objects.push_back(obj);
		--numAliveObjects;
	}
	void release()
	{
		for (auto o : objects) delete o;
		objects.clear();
	}
};

struct Card
{
	unsigned char value;
	unsigned char color;
	bool operator<(const Card& other) const
	{
		return this->value < other.value || (this->value == other.value && this->color < other.color);
	}
	bool operator==(const Card& other) const { return this->value == other.value && this->color == other.color; }
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

template <size_t SIZE>
struct CardSet
{
	static const int not_found = -1;
	Card cards[SIZE];
	size_t numCards;
	CardSet() : numCards(0) {}
	CardSet(const CardSet& other)
	{
		copy(other, other.numCards);
	}
	CardSet(std::initializer_list<Card> initial)
	{
		numCards = 0;
		for (auto & c : initial) append(c);
	}
	void append(const Card & c)
	{
		cards[numCards++] = c;
	}
	void remove(const Card & c)
	{
		auto it = std::remove(begin(), end(), c);
		if (it != end()) --numCards;
	}
	void sort()
	{
		std::sort(begin(), end());
	}
	size_t find(const Card& c) const
	{
		auto it = std::find(begin(), end(), c);
		return it != end() ? it - begin() : not_found;
	}
	wstring toString() const
	{
		if (0 == numCards) return wstring();
		std::wstringstream ss;
		for (int i = 0; i < numCards; ++i) {
			ss << cards[i].toString() << L" ";
		}
		return ss.str();
	}
	static CardSet createUsingAllCards()
	{
		CardSet allcards;
		int idx = 0;
		for (unsigned char v = 9; v <= 14; ++v)
			for (unsigned char c = 0; c < 4; ++c, ++idx)
			{
				allcards.cards[idx] = { v,c };
			}
		allcards.numCards = 24;
		return allcards;
	}
	CardSet& shuffle(unsigned seed)
	{
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, 23);
		generator.seed(seed);
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
		for (auto i = 0; i < numCards; ++i)
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
					vqtp.append({ last_value,0 });
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
	void copy(const CardSet& set, size_t numCardsToCopy)
	{
		std::copy(set.begin(), set.cards + numCardsToCopy, begin());
		numCards = numCardsToCopy;
	}

	const Card& first() const { return cards[0]; }
	const Card& last()  const { return cards[numCards - 1]; }
	Card* begin() { return cards; }
	const Card* begin() const { return cards; }
	Card* end() { return cards + numCards; }
	const Card* end() const { return cards + numCards; }
	Card& operator[](size_t idx) { return cards[idx]; }
	const Card& operator[](size_t idx) const { return cards[idx]; }
};

using Stack = CardSet<24>;
using Hand  = CardSet<24>;

struct GameState;
struct BaseMove : IMove
{
	virtual GameState* apply(const GameState*cs, int player) const = 0;
};

struct GameState : IGameState
{
	static const Card firstToPlay;
	static const Hand allCardsSorted;

	Stack stack;
	Hand hand[4];
	int numPlayers;
	int currPlayer;
	
	GameState() : m_refCnt(1)
	{}
	void Initialize(int numPlayers, unsigned seed) override
	{
		Hand allCards = Hand::createUsingAllCards().shuffle(seed);
		this->numPlayers = numPlayers;
		const int NumCardsPerPlayer = 24 / numPlayers;
		for (int pn = 0, cn = 0; pn < numPlayers; ++pn)
		{
			auto & phand = hand[pn];
			phand.numCards = 0;
			for (int ci = 0; ci < NumCardsPerPlayer; ++ci) {
				phand.append(allCards[cn++]);
			}
			phand.sort();
		}
		stack.numCards = 0;
		for (int pn = 0; pn < numPlayers; ++pn)
		{
			if (hand[pn].find(firstToPlay) != Hand::not_found) {
				currPlayer = pn;
				break;
			}
		}
		assertNumCards();
	}
	MoveList GetPlayerLegalMoves(int player) const override;
	IGameState* Next(const MoveList& moves) const
	{
		auto  mv = static_cast<BaseMove*>(moves[0]);
		GameState* cs = mv->apply(this, 0);
		GameState* ns = nullptr;
		mv->release();
		for (auto pn = 1; pn < moves.size(); ++pn)
		{
			mv = static_cast<BaseMove*>(moves[pn]);
			ns = mv->apply(cs, pn);
			cs->Release();
			cs = ns;
			mv->release();
		}
		_ASSERT(ns != nullptr);
		ns->nextPlayer();
		return ns;
	}
	wstring ToString() const override
	{
		std::wstringstream ss;
		ss << L"Stack : " << stack.toString() << std::endl;
		for (int i = 0; i < numPlayers; ++i) {
			ss << L"Player " << i << L" hand : " << hand[i].toString() << std::endl;
		}
		ss << L"Current player : " << currPlayer;
		return ss.str();
	}
	bool IsTerminal() const override
	{
		int numPlayersWithCards = 0;
		for (auto pn=0;pn<numPlayers;++pn) {
			if (hand[pn].numCards != 0) ++numPlayersWithCards;
		}
		return 1==numPlayersWithCards;
	}
	void Score(int scoreArr[]) const override
	{
		if (IsTerminal())
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
	static void handToBits(const Hand& set, GameStateHash_t& bs, size_t offset)
	{
		size_t i = 0, j = 0;
		while (i < allCardsSorted.numCards && j < set.numCards)
		{
			if (allCardsSorted[i] == set[j]) {
				bs.set(offset + i);
				++j;
			}
			++i;
		}
	}
	GameStateHash_t Hash() const override
	{
		//GameStateHash_t hash(24 + numPlayers * 24);
		GameStateHash_t hash;
		Hand sorted_stack(stack);
		sorted_stack.sort();
		handToBits(sorted_stack, hash, 0);
		size_t offset = 24;
		for (auto i = 0; i < numPlayers; ++i, offset += 24) {
			handToBits(hand[i], hash, offset);
		}
		return hash;
	}
	string HashS() const override
	{
		//string s;
		//boost::to_string(Hash(), s);
		//returns;
		return Hash().to_string();
	}
	int NumPlayers() const override
	{
		return numPlayers;
	}
	void nextPlayer()
	{
		currPlayer = (currPlayer + 1) % numPlayers;
	}
	IGameState* Apply(const IMove* mv, int playerNum) const override
	{
		auto bmv = static_cast<const BaseMove*>(mv);
		auto ns = bmv->apply(this, playerNum);
		ns->nextPlayer();
		return ns;
	}
	void assertNumCards() const
	{
		auto tot_cards = stack.numCards;
		for (int i = 0; i < numPlayers; ++i) tot_cards += hand[i].numCards;
		_ASSERT(tot_cards == 24);
	}
	mutable int m_refCnt;
	void addRef() const { ++m_refCnt; }
	
	static GameState* create();
	void Release() override;
};


const Card GameState::firstToPlay = { 9, 0 };
const Hand GameState::allCardsSorted = Hand::createUsingAllCards();


struct Noop : BaseMove
{
	static Noop* create()
	{
		static Noop noop;
		return &noop;
	}
	void release() override { }
	wstring toString() const override { return L"noop"; }
	GameState* apply(const GameState* cs, int player) const override 
	{
		cs->addRef();
		return const_cast<GameState*>(cs);
	}
};

struct PlayCard : BaseMove, CardSet<4>
{
	static PlayCard* create(std::initializer_list<Card> initial);
	void release() override;
	PlayCard(){}
	PlayCard(std::initializer_list<Card> initial) : CardSet(initial)
	{}
	void init(std::initializer_list<Card> initial)
	{
		numCards = 0;
		for (auto & c : initial) append(c);
	}
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
		ns->currPlayer = cs->currPlayer;
		ns->stack.copy(cs->stack, cs->stack.numCards);
		for (auto pn=0;pn<cs->numPlayers;++pn) {
			ns->hand[pn].copy(cs->hand[pn], cs->hand[pn].numCards);
		}
		for (auto i=0;i<numCards;++i) {
			auto & c = cards[i];
			ns->stack.append(c);
			ns->hand[player].remove(c);
		}
		ns->assertNumCards();
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
		ns->currPlayer = cs->currPlayer;
		ns->stack.copy(cs->stack, cs->stack.numCards - NUM_CARDS);
		for(auto pn=0;pn<cs->numPlayers;++pn) {
			ns->hand[pn].copy(cs->hand[pn], cs->hand[pn].numCards);
		}

		auto last_idx = cs->stack.numCards - 1;
		for (auto i=0;i<NUM_CARDS;++i)
		{
			ns->hand[player].append(cs->stack.cards[last_idx--]);
		}
		ns->hand[player].sort();
		ns->assertNumCards();
		return ns;
	}
};

MoveList GameState::GetPlayerLegalMoves(int player) const
{
	if (player != currPlayer){
		return { Noop::create() };
	}

	MoveList moves;
	moves.reserve(20);
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
	_ASSERT(!moves.empty());
	return moves;
}

IGameState* createGraWPanaGameState()
{
	return GameState::create();
}

struct MemoryPools
{
	~MemoryPools()
	{
		GameStatePool.release();
		PlayCardMovePool.release();
	}
	ObjectPool<GameState>	GameStatePool;
	ObjectPool<PlayCard>	PlayCardMovePool;
};
DWORD tlsIndex = TLS_OUT_OF_INDEXES;
DWORD tlsIndexRefCnt = 0;
boost::mutex mut;
void initGraWPanaMemoryMgmt()
{
	mut.lock();
	if (0 == tlsIndexRefCnt) {
		tlsIndex = TlsAlloc();
		_ASSERT(tlsIndex != TLS_OUT_OF_INDEXES);
	}
	auto * mp = new MemoryPools;
	TlsSetValue(tlsIndex, (LPVOID)mp);
	++tlsIndexRefCnt;
	mut.unlock();
}
MemoryPools* getMemoryPoolsInst()
{
	return static_cast<MemoryPools*>(TlsGetValue(tlsIndex));
}
void cleanupGraWPanaMemoryMgmt()
{
	mut.lock();
	auto *mp = getMemoryPoolsInst();
	_ASSERT(0 == mp->GameStatePool.numAliveObjects);
	_ASSERT(0 == mp->PlayCardMovePool.numAliveObjects);
	delete mp;
	TlsSetValue(tlsIndex, (LPVOID)nullptr);
	if (0 == --tlsIndexRefCnt) {
		TlsFree(tlsIndex);
		tlsIndex = TLS_OUT_OF_INDEXES;
	}
	mut.unlock();
}
GameState* GameState::create()
{
	auto gs = getMemoryPoolsInst()->GameStatePool.alloc();
	gs->m_refCnt = 1;
	return gs;
}
PlayCard* PlayCard::create(std::initializer_list<Card> cards)
{
	auto mv = getMemoryPoolsInst()->PlayCardMovePool.alloc();
	mv->init(cards);
	return mv;
}
void PlayCard::release()
{
	getMemoryPoolsInst()->PlayCardMovePool.free(this);
}
void GameState::Release()
{
	if (--m_refCnt == 0)
	{
		getMemoryPoolsInst()->GameStatePool.free(this);
	}
}
