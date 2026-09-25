#pragma once

class GameDataHolder;

class GameDataHolderAccessor {
public:
    const GameDataHolder* mData;
    GameDataHolderAccessor(const GameDataHolder* holder = nullptr) : mData(holder) {}
};
