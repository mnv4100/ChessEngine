#include "Io.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
    constexpr float BOARD_MARGIN = 12.0f;
    constexpr float MOVE_MARKER_RADIUS_RATIO = 0.18f;

    std::string squareToNotation(const Vec2 &pos)
    {
        char file = static_cast<char>('a' + pos.x);
        char rank = static_cast<char>('8' - pos.y);
        return {file, rank};
    }

    char pieceToSymbol(uint8_t piece)
    {
        switch (static_cast<PIECE>(piece))
        {
        case PIECE::King:
            return 'K';
        case PIECE::Queen:
            return 'Q';
        case PIECE::Rook:
            return 'R';
        case PIECE::Bishop:
            return 'B';
        case PIECE::Knight:
            return 'N';
        case PIECE::Pion:
            return 'P';
        }
        return '?';
    }

    ImVec2 centerRect(const ImVec2 &min, float size)
    {
        return ImVec2{min.x + size * 0.5f, min.y + size * 0.5f};
    }
}

Io::Io()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialise GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    //glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#ifdef GLFW_SCALE_TO_MONITOR
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif

    window = glfwCreateWindow(windowSizeX, windowSizeY, "Super Chess Engine", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();    
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialise OpenGL loader");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);

#if defined(__APPLE__)
    constexpr const char *glslVersion = "#version 150";
#else
    constexpr const char *glslVersion = "#version 330";
#endif
    ImGui_ImplOpenGL3_Init(glslVersion);

    boardCellSize = static_cast<float>(cellSize);

    if (!loadPieceSprites())
    {
        throw std::runtime_error("Failed to load chess piece sprites");
    }
}

Io::~Io()
{
    destroyPieceSprites();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Io::beginFrame()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    buildDockspace();
}

void Io::endFrame()
{
    ImGui::Render();
    int displayW = 0;
    int displayH = 0;
    glfwGetFramebufferSize(window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }
    glfwSwapBuffers(window);
}

bool Io::shouldClose() const
{
    return glfwWindowShouldClose(window) != 0;
}

void Io::buildDockspace()
{
    ImGuiIO &io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
    {
        return;
    }

    constexpr ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoNavFocus;

    if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)
    {
        windowFlags |= ImGuiWindowFlags_NoBackground;
    }

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});

    if (ImGui::Begin("DockSpaceRoot", nullptr, windowFlags))
    {
        ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspaceId, ImVec2{0.0f, 0.0f}, dockspaceFlags);
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
}

std::optional<SIDE> Io::renderSideSelectionPrompt()
{
    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 windowSize{ 360.0f, 200.0f };
    std::optional<SIDE> selection;
    // const ImVec2 center{ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f };

    // const ImVec2 center { windowSizeX * 0.5f, windowSizeY * 0.5f };

    // ImGui::SetNextWindowPos(ImVec2{ center.x - windowSize.x * 0.5f, center.y - windowSize.y * 0.5f }, ImGuiCond_Appearing);
	// ImGui::SetNextWindowPos(ImVec2{ center.x , center.y }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

   
    if (ImGui::Begin("Choose your side", 
        nullptr
		//, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings

    ))
    {
        ImGui::Spacing();
        ImGui::TextWrapped("Choose your side");
        ImGui::Spacing();

        if (ImGui::Button("Play as White", ImVec2{ -FLT_MIN, 0.0f }))
        {
            selection = SIDE::WHITE_SIDE;
        }
        if (ImGui::Button("Play as Black", ImVec2{ -FLT_MIN, 0.0f }))
        {
            selection = SIDE::BLACK_SIDE;
        }
        if (ImGui::Button("Watch AI vs AI", ImVec2{ -FLT_MIN, 0.0f }))
        {
            selection = SIDE::SPECTATOR_SIDE;
        }
    }
    ImGui::End();

    return selection;
}

void Io::renderChessBoard(Core &core,
                          const Vec2 *checkedKingPos,
                          const std::vector<std::string> &moveHistory,
                          const Vec2 *selectedCell)
{
    boardLayoutValid = false;

    constexpr float WINDOW_SIZE = 900.0f;
    constexpr float BOARD_PIXEL_SIZE = 800.0f;
    constexpr float PADDING = (WINDOW_SIZE - BOARD_PIXEL_SIZE) / 2.0f;

    ImGui::SetNextWindowSize(ImVec2{WINDOW_SIZE, WINDOW_SIZE}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2{50.0f, 50.0f}, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Chess Board", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        const ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
        const ImVec2 startCursor = ImGui::GetCursorScreenPos();
        boardOrigin = ImVec2{startCursor.x + PADDING, startCursor.y + PADDING};

        ImDrawList *drawList = ImGui::GetWindowDrawList();
        const ImU32 lightColor = ImGui::GetColorU32(ImVec4{0.86f, 0.86f, 0.86f, 1.0f});
        const ImU32 darkColor = ImGui::GetColorU32(ImVec4{0.32f, 0.32f, 0.32f, 1.0f});
        const ImU32 selectedColor = ImGui::GetColorU32(ImVec4{0.85f, 0.65f, 0.22f, 1.0f});
        const ImU32 checkColor = ImGui::GetColorU32(ImVec4{0.78f, 0.18f, 0.18f, 1.0f});
        const ImU32 borderColor = ImGui::GetColorU32(ImVec4{0.08f, 0.08f, 0.08f, 1.0f});
        const ImU32 moveColor = ImGui::GetColorU32(ImVec4{0.1f, 0.45f, 0.89f, 0.85f});

        Vec2 displaySelected{};
        bool hasSelected = false;
        if (selectedCell)
        {
            displaySelected = toDisplayCoordinates(*selectedCell);
            hasSelected = true;
        }

        Vec2 displayChecked{};
        bool hasChecked = false;
        if (checkedKingPos)
        {
            displayChecked = toDisplayCoordinates(*checkedKingPos);
            hasChecked = true;
        }

        for (uint8_t y = 0; y < 8; ++y)
        {
            for (uint8_t x = 0; x < 8; ++x)
            {
                const Vec2 displayCell{x, y};
                const Vec2 boardPos = toBoardCoordinates(displayCell);
                const BoardCell &cell = core.At(boardPos);

                const ImVec2 min{boardOrigin.x + static_cast<float>(x) * boardCellSize,
                                 boardOrigin.y + static_cast<float>(y) * boardCellSize};
                const ImVec2 max{min.x + boardCellSize, min.y + boardCellSize};

                bool isSelected = hasSelected && displaySelected.x == displayCell.x && displaySelected.y == displayCell.y;
                bool isChecked = hasChecked && displayChecked.x == displayCell.x && displayChecked.y == displayCell.y;

                ImU32 color = ((x + y) % 2 == 0) ? lightColor : darkColor;
                if (isSelected)
                {
                    color = selectedColor;
                }
                if (isChecked)
                {
                    color = checkColor;
                }

                drawList->AddRectFilled(min, max, color);
                drawList->AddRect(min, max, borderColor, 0.0f, 0, 1.0f);

                for (const auto &move : possibleMovesToRender)
                {
                    Vec2 displayMove = toDisplayCoordinates(move);
                    if (displayMove.x == displayCell.x && displayMove.y == displayCell.y)
                    {
                        const ImVec2 center = centerRect(min, boardCellSize);
                        drawList->AddCircleFilled(center, boardCellSize * MOVE_MARKER_RADIUS_RATIO, moveColor, 32);
                        break;
                    }
                }

                if (cell.fill == 0)
                {
                    continue;
                }

                if (hasPieceSprites())
                {
                    const auto columnIndex = [piece = static_cast<PIECE>(cell.piece)]() -> int {
                        switch (piece)
                        {
                        case PIECE::King:
                            return 0;
                        case PIECE::Queen:
                            return 1;
                        case PIECE::Bishop:
                            return 2;
                        case PIECE::Knight:
                            return 3;
                        case PIECE::Rook:
                            return 4;
                        case PIECE::Pion:
                            return 5;
                        }
                        return 0;
                    }();

                    const int rowIndex = (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE)) ? 0 : 1;

                    const float uMin = pieceTileSize.x * static_cast<float>(columnIndex) / pieceTextureSize.x;
                    const float vMin = pieceTileSize.y * static_cast<float>(rowIndex) / pieceTextureSize.y;
                    const float uMax = pieceTileSize.x * static_cast<float>(columnIndex + 1) / pieceTextureSize.x;
                    const float vMax = pieceTileSize.y * static_cast<float>(rowIndex + 1) / pieceTextureSize.y;

                    const float padding = boardCellSize * 0.08f;
                    const ImVec2 imageMin{min.x + padding, min.y + padding};
                    const ImVec2 imageMax{max.x - padding, max.y - padding};
                    const ImTextureID textureId = static_cast<ImTextureID>(pieceTexture);
                    drawList->AddImage(textureId, imageMin, imageMax, ImVec2{uMin, vMin}, ImVec2{uMax, vMax});
                }
                else
                {
                    const char symbol = pieceToSymbol(cell.piece);
                    const float fontSize = boardCellSize * 0.55f;
                    const ImU32 textColor = (cell.side == static_cast<uint8_t>(SIDE::WHITE_SIDE))
                                                ? ImGui::GetColorU32(ImVec4{0.95f, 0.95f, 0.95f, 1.0f})
                                                : ImGui::GetColorU32(ImVec4{0.05f, 0.05f, 0.05f, 1.0f});
                    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, &symbol, &symbol + 1);
                    const ImVec2 textPos{min.x + (boardCellSize - textSize.x) * 0.5f,
                                         min.y + (boardCellSize - textSize.y) * 0.5f};
                    drawList->AddText(ImGui::GetFont(), fontSize, textPos, textColor, &symbol, &symbol + 1);
                }
            }
        }

        // File labels
        const ImU32 labelColor = ImGui::GetColorU32(ImVec4{0.9f, 0.9f, 0.9f, 1.0f});
        for (uint8_t x = 0; x < 8; ++x)
        {
            char fileChar = static_cast<char>(whitePerspective ? ('A' + x) : ('H' - x));
            const std::string label(1, fileChar);
            const ImVec2 bottomPos{boardOrigin.x + static_cast<float>(x) * boardCellSize + boardCellSize * 0.4f,
                                   boardOrigin.y + BOARD_PIXEL_SIZE + 8.0f};
            const ImVec2 topPos{bottomPos.x, boardOrigin.y - boardCellSize * 0.35f};
            drawList->AddText(bottomPos, labelColor, label.c_str());
            drawList->AddText(topPos, labelColor, label.c_str());
        }

        // Rank labels
        for (uint8_t y = 0; y < 8; ++y)
        {
            char rankChar = static_cast<char>(whitePerspective ? ('8' - y) : ('1' + y));
            const std::string label(1, rankChar);
            const ImVec2 leftPos{boardOrigin.x - boardCellSize * 0.4f,
                                 boardOrigin.y + static_cast<float>(y) * boardCellSize + boardCellSize * 0.4f};
            const ImVec2 rightPos{boardOrigin.x + BOARD_PIXEL_SIZE + boardCellSize * 0.15f,
                                  leftPos.y};
            drawList->AddText(leftPos, labelColor, label.c_str());
            drawList->AddText(rightPos, labelColor, label.c_str());
        }

        ImGui::Dummy(ImVec2{WINDOW_SIZE, WINDOW_SIZE});
        boardLayoutValid = true;
    }
    ImGui::End();

    if (ImGui::Begin("Move History", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        const size_t maxDisplayed = 32;
        const size_t totalMoves = moveHistory.size();
        const size_t startIndex = (totalMoves > maxDisplayed) ? totalMoves - maxDisplayed : 0U;

        for (size_t i = startIndex; i < totalMoves; ++i)
        {
            const bool whiteMove = (i % 2 == 0);
            const size_t moveNumber = i / 2 + 1;
            std::string entry = std::to_string(moveNumber);
            entry += whiteMove ? ". " : "... ";
            entry += moveHistory[i];
            ImGui::TextUnformatted(entry.c_str());
        }
    }
    ImGui::End();
}

void Io::renderGameInfo(SIDE toMove,
                        SIDE humanSide,
                        bool aiTurn,
                        bool aiVsAi,
                        const std::string &statusMessage,
                        const Vec2 *selectedCell,
                        bool hasSelection)
{
    if (ImGui::Begin("Game Info", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("Current turn: %s", toMove == SIDE::WHITE_SIDE ? "White" : "Black");
        ImGui::Text("Human plays: %s", humanSide == SIDE::WHITE_SIDE ? "White" : "Black");
        ImGui::Text("Mode: %s", aiVsAi ? "AI vs AI" : "Human vs AI");
        ImGui::Text("Active side: %s", aiTurn ? "AI" : "Human");
        ImGui::Separator();
        ImGui::TextWrapped("%s", statusMessage.c_str());

        if (hasSelection && selectedCell)
        {
            ImGui::Text("Selected: %s", squareToNotation(*selectedCell).c_str());
        }
        else
        {
            ImGui::TextUnformatted("Selected: --");
        }
    }
    ImGui::End();
}

bool Io::getOveredCell(Vec2 &cell) const
{
    if (!boardLayoutValid)
    {
        return false;
    }

    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 mouse = io.MousePos;
    const float boardPixelSize = boardCellSize * 8.0f;
    if (mouse.x < boardOrigin.x || mouse.y < boardOrigin.y ||
        mouse.x >= boardOrigin.x + boardPixelSize || mouse.y >= boardOrigin.y + boardPixelSize)
    {
        return false;
    }

    int ix = static_cast<int>((mouse.x - boardOrigin.x) / boardCellSize);
    int iy = static_cast<int>((mouse.y - boardOrigin.y) / boardCellSize);
    ix = std::clamp(ix, 0, 7);
    iy = std::clamp(iy, 0, 7);

    Vec2 displayCell{static_cast<uint8_t>(ix), static_cast<uint8_t>(iy)};
    cell = toBoardCoordinates(displayCell);
    return true;
}

bool Io::consumeBoardClick(Vec2 &cell) const
{
    if (!boardLayoutValid)
    {
        return false;
    }

    if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        return false;
    }

    return getOveredCell(cell);
}

Vec2 Io::toBoardCoordinates(const Vec2 &displayCell) const
{
    if (whitePerspective)
    {
        return displayCell;
    }
    return Vec2{static_cast<uint8_t>(7 - displayCell.x), static_cast<uint8_t>(7 - displayCell.y)};
}

Vec2 Io::toDisplayCoordinates(const Vec2 &boardCell) const
{
    if (whitePerspective)
    {
        return boardCell;
    }
    return Vec2{static_cast<uint8_t>(7 - boardCell.x), static_cast<uint8_t>(7 - boardCell.y)};
}

bool Io::loadPieceSprites()
{
    destroyPieceSprites();

    const std::string texturePath = std::string(ASSETS_PATH) + "Chess_Pieces_Sprite.svg.png";

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels || width <= 0 || height <= 0)
    {
        if (pixels)
        {
            stbi_image_free(pixels);
        }
        return false;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0)
    {
        stbi_image_free(pixels);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    GLint previousAlignment = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);

    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);

    pieceTexture = texture;
    pieceTextureSize = ImVec2{static_cast<float>(width), static_cast<float>(height)};
    pieceTileSize = ImVec2{pieceTextureSize.x / 6.0f, pieceTextureSize.y / 2.0f};

    return true;
}

void Io::destroyPieceSprites()
{
    if (pieceTexture != 0)
    {
        glDeleteTextures(1, &pieceTexture);
        pieceTexture = 0;
    }
    pieceTextureSize = ImVec2{0.0f, 0.0f};
    pieceTileSize = ImVec2{0.0f, 0.0f};
}
