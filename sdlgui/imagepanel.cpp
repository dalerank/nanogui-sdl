/*
    sdlgui/imagepanel.cpp -- Image panel widget which shows a number of
    square-shaped icons

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/imagepanel.h>

NAMESPACE_BEGIN(sdlgui)

ImagePanel::ImagePanel(Widget *parent)
    : Widget(parent), mThumbSize(64), mSpacing(10), mMargin(10),
  mMouseIndex(-1) 
{
}

Vector2i ImagePanel::gridSize() const
{
    int nCols = 1 + std::max(0,
        (int) ((mSize.x - 2 * mMargin - mThumbSize) /
        (float) (mThumbSize + mSpacing)));
    int nRows = ((int) mImages.size() + nCols - 1) / nCols;
    return Vector2i(nCols, nRows);
}

int ImagePanel::indexForPosition(const Vector2i &p) const
{
  Vector2f pp = (p.tofloat() - Vector2f(mMargin, mMargin)) / (float)(mThumbSize + mSpacing);
    float iconRegion = mThumbSize / (float)(mThumbSize + mSpacing);
    bool overImage = pp.x - std::floor(pp.x) < iconRegion &&
                    pp.y - std::floor(pp.y) < iconRegion;
    Vector2i gridPos = pp.toint();
    Vector2i grid = gridSize();
    overImage &= gridPos.positive() && gridPos.lessOrEq(grid);
    return overImage ? (gridPos.x + gridPos.y * grid.x) : -1;
}

bool ImagePanel::mouseMotionEvent(const Vector2i &p, const Vector2i & /* rel */,
                              int /* button */, int /* modifiers */) 
{
    mMouseIndex = indexForPosition(p);
    return true;
}

bool ImagePanel::mouseButtonEvent(const Vector2i &p, int /* button */, bool down,
                                  int /* modifiers */) 
{
    int index = indexForPosition(p);
    if (index >= 0 && mCallback && down)
        mCallback(index);
    return true;
}

Vector2i ImagePanel::preferredSize(SDL_Renderer *) const
{
  Vector2i grid = gridSize();
    return{
        grid.x * mThumbSize + (grid.x - 1) * mSpacing + 2 * mMargin,
        grid.y * mThumbSize + (grid.y - 1) * mSpacing + 2 * mMargin
    };
}

void ImagePanel::draw(SDL_Renderer* renderer) 
{
  Vector2i grid = gridSize();

    int ax = getAbsoluteLeft();
    int ay = getAbsoluteTop();

    PntRect clip = getAbsoluteCliprect();
    SDL_Rect clipRect = pntrect2srect(clip);

    for (size_t i=0; i<mImages.size(); ++i) 
    {
      Vector2i p = Vector2i(mMargin, mMargin) + Vector2i((int) i % grid.x, (int) i / grid.x) * (mThumbSize + mSpacing);
        p += Vector2i(ax, ay);
        int imgw = mImages[i].w;
        int imgh = mImages[i].h;

        float iw, ih, ix, iy;
        if (imgw < imgh) 
        {
            iw = mThumbSize;
            ih = iw * (float)imgh / (float)imgw;
            ix = 0;
            iy = -(ih - mThumbSize) * 0.5f;
        } 
        else 
        {
            ih = mThumbSize;
            iw = ih * (float)imgw / (float)imgh;
            ix = -(iw - mThumbSize) * 0.5f;
            iy = 0;
        }

        //, 0, mImages[i].first, mMouseIndex == (int)i ? 1.0 : 0.7);

        SDL_Color c{ 0, 0, 0, 128 };
        SDL_Rect shadowPaintRect{ p.x - 1, p.y, mThumbSize + 2, mThumbSize + 2 };

        shadowPaintRect = clip_rects(shadowPaintRect, clipRect);

        if (shadowPaintRect.w > 0 && shadowPaintRect.h > 0)
        {
          SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
          SDL_RenderFillRect(renderer, &shadowPaintRect);
        }

        SDL_Rect imgPaintRect{ p.x + ix, p.y + iy, iw, ih };
        SDL_Rect imgSrcRect{ 0, 0, imgw, imgh };
        PntRect imgrect = clip_rects(srect2pntrect(imgPaintRect), clip);
        imgPaintRect.w = imgrect.x2 - imgrect.x1;
        imgPaintRect.h = imgrect.y2 - imgrect.y1;
        if (imgPaintRect.y < clip.y1)
        {
          imgPaintRect.y = clip.y1;
          imgSrcRect.h = (imgPaintRect.h / (float)ih) * imgh;
          imgSrcRect.y = (1 - (imgPaintRect.h / (float)ih)) * imgh;
        }
        else if(imgPaintRect.h < ih)
        {
          imgSrcRect.h = (imgPaintRect.h / (float)ih) * imgh;
        }

        SDL_RenderCopy(renderer, mImages[i].tex, &imgSrcRect, &imgPaintRect);

        SDL_Rect brect{ p.x + 1, p.y + 1, mThumbSize - 2, mThumbSize - 2};
        brect = clip_rects(brect, clipRect);
        if (brect.w > 0 && brect.h > 0)
        {
          SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 80);
          SDL_RenderDrawRect(renderer, &brect);
        }
    }

    Widget::draw(renderer);
}

NAMESPACE_END(sdlgui)
