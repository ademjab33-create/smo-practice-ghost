#pragma once

class GameDataHolder;

class GameDataHolderWriter {
public:
    GameDataHolder* mData;
    GameDataHolderWriter(GameDataHolder* holder = nullptr) : mData(holder) {}
};
