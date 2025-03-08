/// Helpers for drawing shit.

// Draws a rectangle with the given top-left 2D coordinate with given
// width and height, on the given SFML window, with the given RGB
// color.
void DrawRect(sf::RenderWindow &window, u32 topLeftX, u32 topLeftY, u32 w, u32 h,
							u32 r, u32 g, u32 b)
{
	sf::RectangleShape rect({(f32)w, (f32)h});
	rect.setRotation(0);
	rect.setFillColor(sf::Color(r, g, b));
	rect.setPosition(topLeftX, topLeftY);
	window.draw(rect);
}

sf::Font font("arial.ttf");
// Draws the given text on the window at the given top-left 2D coordinate.
void DrawText(sf::RenderWindow &window, char *text, u32 topLeftX, u32 topLeftY)
{
	// TODO: Fill in this.
}
