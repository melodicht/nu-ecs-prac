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

// Draws the given text on the window at the given top-left 2D coordinate.
void DrawText(sf::RenderWindow &window, char *text, u32 topLeftX, u32 topLeftY)
{
	// TODO: Fill in this.
	sf::Font font;
	if (!font.loadFromFile("main.ttf"))
	{
    // error...
	}
	else
	{
		/*
		sf::Text text(font); 
		// set the string to display
		text.setString("Hello world");

		// set the character size
		text.setCharacterSize(24); // in pixels, not points!

		// set the color
		text.setFillColor(sf::Color::Red);

		// set the text style
		text.setStyle(sf::Text::Bold | sf::Text::Underlined);

		...

			// inside the main loop, between window.clear() and window.display()
			window.draw(text);
		*/
	}
	
}
