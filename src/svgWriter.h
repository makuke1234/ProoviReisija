#ifndef SVG_WRITER_H
#define SVG_WRITER_H

#include "dataModel.h"

#include <stdio.h>

/**
 * @brief Data structure for RGB colors in SVG file
 * 
 */
typedef struct svgRGB
{
	uint8_t r, g, b, a;

} svgRGB_t;

#define SVG_WIDTH  ((size_t)1920)
#define SVG_HEIGHT ((size_t)1080)
#define SVG_MARGIN ((size_t)20)

#define SVG_LINE_STROKE  ((size_t)3)
#define SVG_POINT_RADIUS ((size_t)20)
#define SVG_FONT_SIZE    ((size_t)32)


/**
 * @brief Creates a svgRGB structure from 3 color components, alpha is max
 * 
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @return svgRGB_t svgRGB structure
 */
svgRGB_t svg_rgb(uint8_t r, uint8_t g, uint8_t b);
/**
 * @brief Creates a svgRGB structure from 3 color compoenent + alpha
 * 
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 * @return svgRGB_t svgRGB structure
 */
svgRGB_t svg_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
/**
 * @brief Creates a svgRGB structure from single 32-bit unsigned integer
 * representing RGB hex code
 * 
 * @param color RGB hex code
 * @return svgRGB_t svgRGB structure
 */
svgRGB_t svg_rgba32(uint32_t color);

/**
 * @brief Initialises necessary settings for SVG to work
 * 
 */
void svg_init(void);
/**
 * @brief Sets the point size to draw into the SVG
 * 
 * @param radius Points radius
 */
void svg_setPointRadius(size_t radius);
/**
 * @brief Sets the font size to draw into the SVG
 * 
 * @param sz Font size
 */
void svg_setFontSize(size_t sz);

/**
 * @brief Writes the SVG file header to file.
 * 
 * @param fp output file pointer
 * @param x x-coordinate of the viewbox
 * @param y y-coordinate of the viewbox
 * @param width Width of the image
 * @param height Height of the image
 * @param backColor Background color of the image
 * @return true Success
 * @return false Failure
 */
bool svg_header(FILE * restrict fp, int64_t x, int64_t y, size_t width, size_t height, svgRGB_t backColor);

/**
 * @brief Draws just a line in the SVG file
 * 
 * @param fp output file pointer
 * @param l Pointer to line
 * @param color The RGB color of the line
 * @return true Success
 * @return false Failure
 */
bool svg_line(FILE * restrict fp, const line_t * restrict l, svgRGB_t color);
/**
 * @brief 
 * 
 * @param fp output file pointer
 * @param l Pointer to line
 * @param color The RGB color of the line & the point
 * @param drawSrc Boolean variable that determines whether the source point of the line should
 * be drawn or not
 * @return true Success
 * @return false Failure
 */
bool svg_linePoint(FILE * restrict fp, const line_t * restrict l, svgRGB_t color, bool drawSrc);
/**
 * @brief Draws a point with its colored circle and text
 * 
 * @param fp output file pointer
 * @param p Pointer to point structure
 * @param color The RGB color of the point
 * @return true Success
 * @return false Failure
 */
bool svg_point(FILE * restrict fp, const point_t * restrict p, svgRGB_t color);
/**
 * @brief Draws middle-aligned text into the SVG
 * 
 * @param fp output file pointer
 * @param x x-coordinate
 * @param y y-coordinate
 * @param str string to draw
 * @return true Success
 * @return false Failure
 */
bool svg_text(FILE * restrict fp, float x, float y, const char * restrict str);

/**
 * @brief Writes the SVG file footer to file.
 * 
 * @param fp output file pointer
 * @return true Success
 * @return false Failure
 */
bool svg_footer(FILE * restrict fp);

#endif
