/**
 * @file arena_view.cc
 * @brief GPU-accelerated, texture-backed Poker AI Arena desktop presentation.
 * @author Arian Mohammadian
 *
 * All graphics are read-only projections of ArenaSnapshot.
 * No poker rule or agent decision is implemented in this file.
 */
#include "arena_view.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <initializer_list>
#include <utility>
#include <string>

#ifndef POKER_ARENA_ASSET_DIR
#define POKER_ARENA_ASSET_DIR "assets/arena"
#endif

namespace arena {
namespace {

constexpr Color kIvory{242, 242, 233, 255};
constexpr Color kText{223, 232, 225, 255};
constexpr Color kDim{139, 159, 155, 255};
constexpr Color kGold{224, 184, 111, 255};
constexpr Color kGoldLow{142, 112, 65, 255};
constexpr Color kAqua{105, 225, 184, 255};
constexpr Color kRed{238, 116, 121, 255};
constexpr Color kBlue{138, 183, 231, 255};
constexpr Color kPanel{16, 27, 33, 244};
constexpr Color kPanel2{23, 39, 44, 250};
constexpr Color kLine{48, 66, 69, 255};
constexpr Color kDark{9, 16, 22, 255};

constexpr Rectangle kNext{43, 810, 175, 64};
constexpr Rectangle kAuto{231, 810, 180, 64};
constexpr Rectangle kNextHand{424, 810, 176, 64};
constexpr Rectangle kCards{613, 810, 187, 64};
constexpr Rectangle kReset{813, 810, 138, 64};
constexpr Rectangle kSlower{1272, 819, 42, 45};
constexpr Rectangle kFaster{1419, 819, 42, 45};
constexpr std::array<std::pair<Rectangle, Command>, 7> kCommands{{
    {kNext, Command::Next}, {kAuto, Command::Auto},
    {kNextHand, Command::NextHand}, {kCards, Command::Cards},
    {kReset, Command::Reset}, {kSlower, Command::Slower},
    {kFaster, Command::Faster}
}};

Font loadFont(const std::initializer_list<const char*>& candidates, bool& owned) {
    for (const char* path : candidates) {
        if (!FileExists(path)) continue;
        Font font = LoadFontEx(path, 64, nullptr, 0);
        if (font.texture.id != 0) {
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            owned = true;
            return font;
        }
    }
    owned = false;
    return GetFontDefault();
}

float widthOf(const Font& font, const std::string& s, float size, float tracking = 0.2f) {
    return MeasureTextEx(font, s.c_str(), size, tracking).x;
}

void type(const Font& font, const std::string& s, float x, float y,
          float size, Color color, float tracking = 0.2f) {
    DrawTextEx(font, s.c_str(), {x, y}, size, tracking, color);
}

void center(const Font& font, const std::string& s, float x, float y,
            float size, Color color, float tracking = 0.2f) {
    type(font, s, x - widthOf(font, s, size, tracking)*.5f, y,
         size, color, tracking);
}

std::string clipText(const Font& font, std::string s, float maxWidth,
                     float size, float tracking = 0.2f) {
    if (widthOf(font, s, size, tracking) <= maxWidth) return s;
    while (!s.empty() && widthOf(font, s + "...", size, tracking) > maxWidth) {
        s.pop_back();
    }
    return s + "...";
}

void rect(Rectangle r, Color color, float roundness = .16f) {
    DrawRectangleRounded(r, roundness, 20, color);
}

void border(Rectangle r, Color color, float thick = 1.0f, float roundness = .16f) {
    DrawRectangleRoundedLinesEx(r, roundness, 20, thick, color);
}

void line(float x1, float y1, float x2, float y2, Color c, float thick=1.0f) {
    DrawLineEx({x1,y1}, {x2,y2}, thick, c);
}

Color withAlpha(Color c, float a) {
    c.a = static_cast<unsigned char>(std::clamp(a, 0.0f, 255.0f));
    return c;
}

float easeOut(float v) {
    const float t = std::clamp(v, 0.0f, 1.0f);
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

bool hovering(Rectangle r) {
    const float scale = std::min(static_cast<float>(GetScreenWidth())/static_cast<float>(kVirtualWidth),
                                 static_cast<float>(GetScreenHeight())/static_cast<float>(kVirtualHeight));
    const Vector2 mouse = GetMousePosition();
    const Vector2 origin{(static_cast<float>(GetScreenWidth())-kVirtualWidth*scale)*.5f,
                         (static_cast<float>(GetScreenHeight())-kVirtualHeight*scale)*.5f};
    return CheckCollisionPointRec({(mouse.x-origin.x)/scale, (mouse.y-origin.y)/scale},r);
}

void drawArt(Texture2D texture, Rectangle dest, Color tint=WHITE) {
    DrawTexturePro(texture,
        {0.0f,0.0f,static_cast<float>(texture.width),static_cast<float>(texture.height)},
        dest,{0.0f,0.0f},0.0f,tint);
}

void chip(float x, float y, float r, Color c, bool rimLight=true) {
    // Material-inspired token: side shadow, faceted edge, weighted center.
    DrawCircleV({x+1.0f,y+3.0f},r+2.0f,{0,0,0,105});
    DrawCircleV({x,y},r+1.0f,{13,21,25,255});
    DrawCircleV({x,y},r,c);
    DrawCircleLines(static_cast<int>(x),static_cast<int>(y),r*.8f,
                    rimLight ? Color{241,231,211,180} : kGoldLow);
    DrawCircleV({x,y},r*.54f,{23,39,43,255});
    DrawCircleV({x,y},r*.45f,c);
    constexpr float tau=6.283185307f;
    for (int i=0;i<8;++i) {
        const float a = tau*static_cast<float>(i)/8.0f;
        line(x+std::cos(a)*r*.79f,y+std::sin(a)*r*.79f,
             x+std::cos(a)*r*.97f,y+std::sin(a)*r*.97f,
             {255,245,221,235},std::max(1.5f,r*.16f));
    }
}

void chipStack(float x, float y, float radius, Color c) {
    chip(x-9,y+4,radius,c);
    chip(x+1,y+1,radius,c);
    chip(x+10,y-4,radius,c);
}

std::string chips(std::int64_t n) {
    std::string str=std::to_string(n);
    const std::size_t begin=(n<0)?1:0;
    for (std::size_t p=str.size();p>begin+3;p-=3) str.insert(p-3,",");
    return str;
}

Color kindColor(int kind) {
    if(kind==1) return kBlue;
    if(kind==2) return kGold;
    if(kind==3) return kAqua;
    return kDim;
}

} // namespace

bool ArenaView::load() {
    fonts_.regular = loadFont({
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        "/usr/share/fonts/opentype/inter/Inter-Regular.otf",
        "/usr/share/fonts/truetype/lato/Lato-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    },fonts_.ownsRegular);
    fonts_.semibold = loadFont({
        "/usr/share/fonts/truetype/inter/Inter-SemiBold.ttf",
        "/usr/share/fonts/opentype/inter/Inter-SemiBold.otf",
        "/usr/share/fonts/truetype/lato/Lato-Semibold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "C:/Windows/Fonts/seguisb.ttf",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
    },fonts_.ownsSemibold);
    fonts_.display = loadFont({
        "/usr/share/fonts/truetype/lato/Lato-Heavy.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "C:/Windows/Fonts/segoeuib.ttf"
    },fonts_.ownsDisplay);

    const auto assetPath = [](const char* name) {
        return (std::filesystem::path(POKER_ARENA_ASSET_DIR) / name).string();
    };
    background_ = LoadTexture(assetPath("backdrop.png").c_str());
    table_ = LoadTexture(assetPath("table.png").c_str());
    cards_ = LoadTexture(assetPath("cards_atlas.png").c_str());
    back_ = LoadTexture(assetPath("card_back.png").c_str());
    if (background_.id == 0 || table_.id == 0 || cards_.id == 0 || back_.id == 0) {
        unload();
        return false;
    }
    for (auto texture : {background_, table_, cards_, back_}) {
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    }
    return true;
}

void ArenaView::unload() {
    if(background_.id) UnloadTexture(background_);
    if(table_.id) UnloadTexture(table_);
    if(cards_.id) UnloadTexture(cards_);
    if(back_.id) UnloadTexture(back_);
    background_={};table_={};cards_={};back_={};
    if (fonts_.ownsRegular) UnloadFont(fonts_.regular);
    if (fonts_.ownsSemibold) UnloadFont(fonts_.semibold);
    if (fonts_.ownsDisplay) UnloadFont(fonts_.display);
    fonts_={};
}

Vector2 ArenaView::toCanvas(Vector2 p) const {
    const float zoom=std::min(static_cast<float>(GetScreenWidth())/static_cast<float>(kVirtualWidth),
                               static_cast<float>(GetScreenHeight())/static_cast<float>(kVirtualHeight));
    if(zoom<=0.0f) return {-100,-100};
    const Vector2 offset{(static_cast<float>(GetScreenWidth())-kVirtualWidth*zoom)*.5f,
                         (static_cast<float>(GetScreenHeight())-kVirtualHeight*zoom)*.5f};
    return {(p.x-offset.x)/zoom,(p.y-offset.y)/zoom};
}

std::optional<Command> ArenaView::hitTest(Vector2 p) const {
    for (const auto& [r,command]:kCommands) {
        if(CheckCollisionPointRec(p,r)) return command;
    }
    return std::nullopt;
}

void ArenaView::drawHeader(const ArenaSnapshot& s) const {
    rect({34,24,54,54},{27,50,52,255},.20f);
    border({34,24,54,54},{125,101,66,210},1.5f,.20f);
    chip(61,50,15,kGold,false);
    type(fonts_.display,"POKER",105,22,30,kIvory,.7f);
    type(fonts_.display,"AI LAB",223,22,30,kGold,.7f);
    type(fonts_.regular,"NO-LIMIT TEXAS HOLD'EM  /  AI COMPETITION ARENA",
         107,62,14,kDim,.9f);
    // Deliberately large whitespace; header reads like a dedicated client.
    rect({1210,30,158,37},{22,44,46,242},.40f);
    DrawCircleV({1231,49},5,s.automatic?kAqua:kGold);
    type(fonts_.semibold,s.automatic?"LIVE MATCH":"SPECTATOR",1245,41,15,kIvory,.2f);
    rect({1382,30,185,37},{23,38,44,245},.30f);
    type(fonts_.regular,"HAND",1400,41,14,kDim,.9f);
    type(fonts_.semibold,"#"+std::to_string(s.handNumber),1464,37,19,kGold,.2f);
    line(33,98,1567,98,{56,71,68,200});
}

void ArenaView::drawTable(const ArenaSnapshot& s) const {
    // The rail/felt/gloss highlights are baked once in a texture to keep WSL/GLFW smooth.
    // Center the felt between two equally tall seats. Overlap with the rail is symmetric.
    drawArt(table_,{35,190,1130,510});
    center(fonts_.semibold,s.street,600,311,16,{197,224,205,210},2.0f);

    chipStack(541,365,12,{193,64,76,255});
    chipStack(571,365,12,{67,128,199,255});
    rect({610,341,185,61},{8,33,30,202},.22f);
    type(fonts_.regular,"TOTAL POT",629,346,13,{190,222,205,255},1.2f);
    type(fonts_.display,chips(s.pot),629,363,30,kIvory,.1f);

    // Cards get dealt from the virtual deck using position/ease, not poker logic.
    constexpr float cardW=82, cardH=118, gap=12;
    constexpr float startX=370, targetY=440;
    for (int i=0;i<5;++i) {
        const Rectangle dest{startX+static_cast<float>(i)*(cardW+gap),targetY,cardW,cardH};
        if (!s.board[i].has_value()) {
            rect(dest,{7,51,42,185},.09f);
            border(dest,{111,163,133,80},1.1f,.09f);
            center(fonts_.semibold,std::to_string(i+1),dest.x+cardW*.5f,
                   dest.y+cardH*.42f,17,{141,185,160,85});
            continue;
        }
        const VisualCard c=*s.board[i];
        if (c.rank<2 || c.rank>14 || c.suit<0 || c.suit>3) continue;
        float dx=dest.x,dy=dest.y;
        float alpha=255.0f;
        if (i>=s.newBoardFrom && s.animationAge<.42f) {
            const float t=easeOut(s.animationAge/.42f);
            dx+=(590-dest.x)*(1.0f-t)*.45f;
            dy-=(55.0f*(1.0f-t));
            alpha=255.0f*(.55f+.45f*t);
        }
        DrawRectangleRounded({dx+3,dy+5,cardW,cardH},.10f,16,{0,0,0,110});
        DrawTexturePro(cards_,{static_cast<float>((c.rank-2)*196),
                               static_cast<float>(c.suit*282),196,282},
                       {dx,dy,cardW,cardH},{0,0},0,withAlpha(WHITE,alpha));
    }

    // Bet badges live entirely on the felt, never underneath either player seat.
    const auto betBadge = [&](int player, float y) {
        const auto amount = s.players[player].committed;
        if (amount <= 0) return;
        constexpr Rectangle badge{877, 0, 241, 42};
        const Rectangle bounds{badge.x, y, badge.width, badge.height};
        rect(bounds,{7,36,32,230},.38f);
        border(bounds,{108,159,132,126},1.0f,.38f);
        chip(bounds.x+23,y+21,11,kGold);
        type(fonts_.semibold,
             clipText(fonts_.semibold,"BET " + chips(amount),192,16),
             bounds.x+46,y+11,16,kIvory,.2f);
    };
    betBadge(1,274);
    betBadge(0,583);

    // Event strip sits in the felt, never over the board.
    rect({340,586,520,43},{8,39,34,209},.55f);
    border({340,586,520,43},{88,140,112,112},1.0f,.55f);
    if (s.handFinished) {
        center(fonts_.semibold,s.result,600,598,17,kGold,.6f);
    } else {
        center(fonts_.semibold,clipText(fonts_.semibold,s.event,480,15),
               600,600,15,kIvory,.3f);
    }
}

void ArenaView::drawSeats(const ArenaSnapshot& s) const {
    // All measurements are in the 1600x900 virtual canvas. The camera applies
    // uniform scaling, so the seats retain their proportions on any window size.
    constexpr float kSeatX=300.0f;
    constexpr float kSeatW=600.0f;
    constexpr float kSeatH=140.0f;
    constexpr float kTopY=120.0f;
    constexpr float kBottomY=630.0f;
    constexpr float kCardW=74.0f;
    constexpr float kCardH=108.0f;

    const auto playerSeat = [&](int player, float y) {
        const VisualPlayer& p=s.players[player];
        const Color accent=p.acting?kAqua:(p.allIn?kGold:kLine);
        const Rectangle panel{kSeatX,y,kSeatW,kSeatH};

        // Raised player panel, with a generous 16px inset around 108px cards.
        rect({panel.x+5,panel.y+9,panel.width,panel.height},
             {1,8,11,148},.12f);
        rect(panel,{14,27,32,252},.12f);
        border(panel,accent,p.acting?2.4f:1.15f,.12f);
        if (p.acting) rect({panel.x,panel.y+24,4,panel.height-48},kAqua,.65f);

        // Identity column: avatar, name/agent, and a separate chip-count row.
        const float avatarX=panel.x+61;
        const float avatarY=y+68;
        DrawCircleV({avatarX,avatarY},32,{35,59,62,255});
        DrawCircleLines(static_cast<int>(avatarX),static_cast<int>(avatarY),33,
                        p.acting?kAqua:Color{75,99,98,255});
        center(fonts_.display,player==0?"P0":"P1",avatarX,y+55,19,
               p.acting?kAqua:kIvory,.2f);

        type(fonts_.semibold,clipText(fonts_.semibold,p.name,240,20,.4f),
             panel.x+114,y+21,20,kIvory,.4f);
        type(fonts_.regular,clipText(fonts_.regular,p.agent,276,14,.3f),
             panel.x+115,y+52,14,kDim,.3f);
        chip(panel.x+129,y+108,13,kGold,false);
        type(fonts_.display,clipText(fonts_.display,chips(p.stack),190,29,.1f),
             panel.x+155,y+85,29,kIvory,.1f);

        // Dealer and action status have their own reserved area to avoid
        // collisions with the player name, stack, or the hole cards.
        if (p.dealer) {
            const float dealerX=panel.x+376;
            const float dealerY=y+106;
            DrawCircleV({dealerX,dealerY},15,{233,226,204,255});
            center(fonts_.display,"D",dealerX,dealerY-10,19,kDark);
        }
        const std::string status=p.folded?"FOLDED":p.allIn?"ALL-IN":p.acting?"TO ACT":"";
        const Color statusColor=p.folded?kRed:p.acting?kAqua:kGold;
        if (!status.empty()) {
            const Rectangle badge{panel.x+293,y+21,111,29};
            rect(badge,{34,58,57,215},.42f);
            center(fonts_.semibold,status,badge.x+badge.width*.5f,
                   badge.y+8,12,statusColor,.3f);
        }

        // Card pair: left/right and top/bottom gaps are identical for both seats.
        // Agent visibility is controlled only by ArenaSnapshot, not by the engine.
        for (int i=0;i<2;++i) {
            const float cx=panel.x+420+static_cast<float>(i)*83.0f;
            const float cy=y+(kSeatH-kCardH)*.5f;
            const Rectangle dst{cx,cy,kCardW,kCardH};
            DrawRectangleRounded({cx+3,cy+4,kCardW,kCardH},
                                 .12f,14,{0,0,0,130});
            const VisualCard& c=p.hole[i];
            if (s.showHoleCards || s.handFinished) {
                if(c.rank>=2 && c.rank<=14 && c.suit>=0 && c.suit<=3) {
                    DrawTexturePro(cards_,
                        {static_cast<float>((c.rank-2)*196),
                         static_cast<float>(c.suit*282),196,282},
                        dst,{0,0},0,WHITE);
                }
            } else {
                drawArt(back_,dst);
            }
        }
    };

    // Align both seats to their respective rails with equal vertical overlap.
    playerSeat(1,kTopY);
    playerSeat(0,kBottomY);
}

void ArenaView::drawSidebar(const ArenaSnapshot& s) const {
    constexpr float x=1184,w=384;
    // SESSION
    rect({x,120,w,214},kPanel,.06f);
    border({x,120,w,214},kLine,1.0f,.06f);
    type(fonts_.semibold,"SESSION OVERVIEW",x+23,140,15,kIvory,1.1f);
    type(fonts_.regular,"LIVE",x+326,143,12,kAqua,1.1f);
    line(x+22,174,x+w-22,174,kLine);
    const std::int64_t total=std::max<std::int64_t>(1,s.players[0].stack+s.players[1].stack+s.pot);
    const auto row=[&](int player,float y,Color tint) {
        const auto& p=s.players[player];
        type(fonts_.regular,"PLAYER "+std::to_string(player),x+24,y,13,kDim,.7f);
        const auto value=chips(p.stack);
        type(fonts_.semibold,value,x+w-25-widthOf(fonts_.semibold,value,17),y-5,17,kIvory);
        rect({x+24,y+29,w-48,8},{34,52,53,255},.55f);
        const float share=std::clamp(static_cast<float>(p.stack)/static_cast<float>(total),0.0f,1.0f);
        if(share>0) rect({x+24,y+29,(w-48)*share,8},tint,.55f);
    };
    row(0,193,kAqua);
    row(1,251,kGold);
    type(fonts_.regular,"POT IN PLAY  "+chips(s.pot),x+24,305,11,kDim,.5f);

    // ACTION HISTORY / real records, scroll by newest; subtle timeline accent.
    rect({x,346,w,407},kPanel,.05f);
    border({x,346,w,407},kLine,1.0f,.05f);
    type(fonts_.semibold,"HAND HISTORY",x+23,368,16,kIvory,.8f);
    type(fonts_.regular,"LATEST FIRST",x+270,372,11,kDim,.5f);
    line(x+22,406,x+w-22,406,kLine);
    const int count=std::min<int>(6,static_cast<int>(s.history.size()));
    if(count==0) center(fonts_.regular,"NO ACTIONS YET",x+w*.5f,515,15,kDim);
    int i=0;
    for(const ActionRecord& rec:s.history) {
        if(i>=count) break;
        const float y=419+static_cast<float>(i)*53.0f;
        const Color ac=kindColor(rec.kind);
        DrawCircleV({x+31,y+11},4.5f,ac);
        if(i<count-1) line(x+31,y+20,x+31,y+52,{50,73,75,220});
        type(fonts_.semibold,clipText(fonts_.semibold,rec.action,310,14),
             x+49,y,14,kIvory,.15f);
        type(fonts_.regular,clipText(fonts_.regular,rec.context,310,11),
             x+49,y+23,11,kDim,.35f);
        ++i;
    }
    // engineering telemetry small, not dominating the product.
    line(x+22,740,x+w-22,740,kLine);
    rect({x,763,w,26},{20,39,41,255},.20f);
    DrawCircleV({x+16,776},4,kAqua);
    type(fonts_.regular,"ENGINE CONNECTED",x+30,770,11,kDim,.4f);
    type(fonts_.regular,std::to_string(s.fps)+" FPS",x+w-65,770,11,kDim,.1f);
}

void ArenaView::drawControls(const ArenaSnapshot& s) const {
    rect({29,795,1542,90},{16,28,34,245},.10f);
    border({29,795,1542,90},{50,67,67,255},1.0f,.10f);
    const auto control=[&](Rectangle b,const std::string& label,
                           const std::string& shortcut,bool primary,bool selected=false) {
        const bool hover=hovering(b);
        const Color base=primary?Color{37,126,99,255}:
                   selected?Color{33,80,70,255}:Color{30,47,52,255};
        rect(b,hover?Color{54,100,88,255}:base,.15f);
        border(b,primary?Color{91,183,147,255}:
               hover?Color{126,148,127,255}:Color{54,77,76,255},1.2f,.15f);
        type(fonts_.semibold,label,b.x+16,b.y+18,15,kIvory,.2f);
        if(!shortcut.empty()) {
            const float sw=widthOf(fonts_.regular,shortcut,12);
            rect({b.x+b.width-16-sw-18,b.y+21,sw+18,24},
                 {10,23,27,180},.22f);
            type(fonts_.regular,shortcut,b.x+b.width-16-sw-9,b.y+26,12,kDim);
        }
    };
    control(kNext,"NEXT","N",true);
    control(kAuto,s.automatic?"PAUSE":"AUTO PLAY","SPACE",false,s.automatic);
    control(kNextHand,"NEXT HAND","M",false);
    control(kCards,s.showHoleCards?"HIDE CARDS":"SHOW CARDS","H",false);
    control(kReset,"RESTART","R",false);
    line(974,813,974,867,kLine);
    type(fonts_.regular,"REPLAY SPEED",1001,811,12,kDim,1.0f);
    type(fonts_.semibold,std::to_string(static_cast<int>(s.secondsPerStep*1000))+" ms",
         1111,842,16,kIvory);
    rect(kSlower,hovering(kSlower)?Color{52,87,83,255}:kPanel2,.18f);
    border(kSlower,kLine,1.0f,.18f);
    rect(kFaster,hovering(kFaster)?Color{52,87,83,255}:kPanel2,.18f);
    border(kFaster,kLine,1.0f,.18f);
    center(fonts_.semibold,"-",kSlower.x+kSlower.width*.5f,kSlower.y+8,22,kIvory);
    center(fonts_.semibold,"+",kFaster.x+kFaster.width*.5f,kFaster.y+8,22,kIvory);
    type(fonts_.regular,"ESC  EXIT",1484,831,12,kDim,.2f);
}

void ArenaView::draw(const ArenaSnapshot& s) const {
    const float scale=std::min(static_cast<float>(GetScreenWidth())/static_cast<float>(kVirtualWidth),
                                static_cast<float>(GetScreenHeight())/static_cast<float>(kVirtualHeight));
    const Vector2 offset{(static_cast<float>(GetScreenWidth())-kVirtualWidth*scale)*.5f,
                         (static_cast<float>(GetScreenHeight())-kVirtualHeight*scale)*.5f};
    BeginDrawing();
    ClearBackground(kDark);
    BeginMode2D(Camera2D{offset,{0,0},0.0f,scale});
    drawArt(background_,{0,0,static_cast<float>(kVirtualWidth),
                           static_cast<float>(kVirtualHeight)});
    drawHeader(s);
    drawTable(s);
    drawSeats(s);
    drawSidebar(s);
    drawControls(s);
    if(!s.error.empty()) {
        rect({340,103,840,42},{120,35,46,250},.15f);
        center(fonts_.semibold,clipText(fonts_.semibold,s.error,820,15),
               760,114,15,kIvory);
    }
    EndMode2D();
    EndDrawing();
}

} // namespace arena
