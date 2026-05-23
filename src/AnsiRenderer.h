#ifndef CC3K_ANSIRENDERER_H
#define CC3K_ANSIRENDERER_H

#include <iosfwd>
#include "Renderer.h"

namespace cc3k {

// IRenderer implementation that writes ANSI-coloured text to a std::ostream
// (default: std::cout). Output is byte-for-byte compatible with the original
// Board::printBoard / displayBoard + Game::renderInfo prints.
class AnsiRenderer : public IRenderer {
public:
    explicit AnsiRenderer(std::ostream& out);

    void drawInitialBoard(const Board& board) override;
    void drawBoard(const Board& board) override;
    void drawHud(const HudInfo& info) override;

private:
    std::ostream& out_;
};

} // namespace cc3k

#endif
