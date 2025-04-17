/// Helpers for drawing shit.

// Draws a fully opaque rectangle with the given top-left 2D coordinate with given
// width and height, on the given SDL_Renderer, with the given RGB
// color. 
void DrawRect(SDL_Renderer *renderer, u32 topLeftX, u32 topLeftY, u32 w, u32 h,
							u32 r, u32 g, u32 b)
{
    SDL_Rect rect;
    rect.x = topLeftX;
    rect.y = topLeftY;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

// Source: https://gist.github.com/Gumichan01/332c26f6197a432db91cc4327fcabb1c
int DrawOutlinedCricle(SDL_Renderer* renderer, int x, int y, int radius, u32 r, u32 g, u32 b)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    while (offsety >= offsetx) {
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
        status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
        status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
        status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}

// source same as above
int DrawFilledCircle(SDL_Renderer * renderer, int x, int y, int radius, u32 r, u32 g, u32 b)
{
    int offsetx, offsety, d;
    int status;

    offsetx = 0;
    offsety = radius;
    d = radius -1;
    status = 0;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    while (offsety >= offsetx) {

        status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                     x + offsety, y + offsetx);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                     x + offsetx, y + offsety);
        status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                     x + offsetx, y - offsety);
        status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                     x + offsety, y - offsetx);

        if (status < 0) {
            status = -1;
            break;
        }

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }

    return status;
}


// Draws the given text on the window at the given top-left 2D coordinate.
// TODO: Change to SDL
void DrawText(SDL_Renderer* renderer, char *text, u32 topLeftX, u32 topLeftY)
{
}
