//
// Created by Pc on 05/11/2025.
//

#ifndef CHESSENGINEPROJECT_CORE_H
#define CHESSENGINEPROJECT_CORE_H

#include <cstdint>
#include <array>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "definition.h"


class Core {

public:
	explicit Core();
	~Core() = default;

	void debugDisplayChessBoard() const;
	bool isMoveLegal(const Vec2& from, const Vec2& to) const;
    bool isPathClear(const Vec2& from, const Vec2& to) const;
    bool movePiece(const Vec2& from, const Vec2& to);
    [[nodiscard]] bool isKingInCheck(SIDE kingSide) const;
    std::vector<Vec2> getPossibleMoves(const Vec2& from) const;
    [[nodiscard]] Vec2 findKing(SIDE side) const;


    void setupCache();
        void updateCache(const Vec2& from,
            const Vec2& to,
            bool capturedDestination,
            std::optional<Vec2> enPassantCaptured,
            std::optional<std::pair<Vec2, Vec2>> rookMove);

    void renewCache() {};


    // setup cache
    // update cache 
    // renew cache

    std::vector<Vec2> filledCell;


    // move generation 

	// make a fake move and see if we can get 
    // 





	[[nodiscard]] const BoardCell& At(const Vec2& pos) const { return chessBoard[pos.y * 8 + pos.x];  };
        [[nodiscard]] BoardCell& At(const Vec2& pos) { return chessBoard[pos.y * 8 + pos.x]; };

private:
        void removeFromCache(const Vec2& pos);

        void fillChessBoard();
        
        [[nodiscard]] inline bool isMoveInBounds(const Vec2& cell) const;

        bool isSquareAttacked(const Vec2& square, SIDE bySide) const;

        bool hasRookMoved(SIDE side, bool kingSide) const;
        void markRookMoved(SIDE side, bool kingSide);
        void handleRookCapture(const Vec2& pos, SIDE capturedSide);

        constexpr inline BoardCell makeCell(PIECE p, SIDE s, bool occupied) noexcept;

        // std::map<SIDE, std::map<PIECE, uint8_t>> takenPiecesCount;

        bool whiteKingMoved{ false };
        bool blackKingMoved{ false };
        bool whiteRookMoved[2]{ false, false };
        bool blackRookMoved[2]{ false, false };

        bool enPassantActive{ false };
        Vec2 enPassantTarget{ 0, 0 };
        Vec2 enPassantCapturedPawn{ 0, 0 };


        // 1D array to use full one line of cache 64 bits
        alignas(64) BoardCell chessBoard[64]{};
};


#endif //CHESSENGINEPROJECT_CORE_H