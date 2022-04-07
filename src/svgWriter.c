#include "svgWriter.h"

struct svgSettings
{
	size_t pRadius, fontSize;

	double scalex, scaley;
};

static struct svgSettings svgSettings;


svgRGB_t svg_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return (svgRGB_t){
		.r = r,
		.g = g,
		.b = b,
		.a = 0xFF
	};
}
svgRGB_t svg_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return (svgRGB_t){
		.r = r,
		.g = g,
		.b = b,
		.a = a
	};
}
svgRGB_t svg_rgba32(uint32_t color)
{
	return (svgRGB_t){
		.r = (uint8_t)((color >> 24) & 0xFF),
		.g = (uint8_t)((color >> 16) & 0xFF),
		.b = (uint8_t)((color >>  8) & 0xFF),
		.a = (uint8_t)( color        & 0xFF)
	};
}


void svg_init(void)
{
	svgSettings = (struct svgSettings){
		.pRadius  = SVG_POINT_RADIUS,
		.fontSize = SVG_FONT_SIZE
	};
}
void svg_setPointRadius(size_t radius)
{
	svgSettings.pRadius = radius;
}
void svg_setFontSize(size_t sz)
{
	svgSettings.fontSize = sz;
}

bool svg_header(FILE * restrict fp, int64_t x, int64_t y, size_t width, size_t height, svgRGB_t backColor)
{
	assert(width  > 0);
	assert(height > 0);

	x -= SVG_MARGIN;
	y += SVG_MARGIN;
	width  += SVG_MARGIN * 2;
	height += SVG_MARGIN * 2;
	
	svgSettings.scalex = (double)SVG_WIDTH  / (double)width;
	svgSettings.scaley = (double)SVG_HEIGHT / (double)height;
	const int64_t vx = (int64_t)((double)x * svgSettings.scalex);
	const int64_t vy = (int64_t)((double)y * svgSettings.scalex);

	size_t newHeight = (size_t)(svgSettings.scalex / svgSettings.scaley * (double)SVG_HEIGHT);
	svgSettings.scaley = svgSettings.scalex;

	return fprintf(
		fp,
		"<svg version=\"1.1\" width=\"%zu\" height=\"%zu\" viewBox=\"%lld %lld %zu %zu\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<rect x=\"%lld\" y=\"%lld\" width=\"100%%\" height=\"100%%\" fill=\"rgb(%hu, %hu, %hu)\" />\n\n",
		SVG_WIDTH, newHeight,
		vx, -vy, SVG_WIDTH, newHeight,
		vx, -vy,
		(uint16_t)backColor.r, (uint16_t)backColor.g, (uint16_t)backColor.b
	) > 0;
}

bool svg_line(FILE * restrict fp, const line_t * restrict l, svgRGB_t color)
{
	assert(fp     != NULL);
	assert(l      != NULL);
	assert(l->src != NULL);
	assert(l->dst != NULL);

	return fprintf(
		fp,
		"<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" style=\"stroke:rgb(%hu, %hu, %hu);stroke-width:%zu\" />\n",
		(double)l->src->x * svgSettings.scalex, -(double)l->src->y * svgSettings.scaley,
		(double)l->dst->x * svgSettings.scalex, -(double)l->dst->y * svgSettings.scaley,
		(uint16_t)color.r, (uint16_t)color.g, (uint16_t)color.b,
		SVG_LINE_STROKE
	) > 0;
}
bool svg_linePoint(FILE * restrict fp, const line_t * restrict l, svgRGB_t color, bool drawSrc)
{
	assert(fp != NULL);
	assert(l  != NULL);
	assert(l->src != NULL);
	assert(l->dst != NULL);

	return svg_line(fp, l, color) && ((drawSrc && svg_point(fp, l->src, color)) || !drawSrc) && svg_point(fp, l->dst, color);
}
bool svg_point(FILE * restrict fp, const point_t * restrict p, svgRGB_t color)
{
	assert(fp != NULL);
	assert(p  != NULL);
	assert(p->id.str != NULL);

	const double x = (double)p->x * svgSettings.scalex, y = -(double)p->y * svgSettings.scaley;
	
	return fprintf(
		fp,
		"<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%zu\" fill=\"rgb(%hu, %hu, %hu)\" />\n"
		"<text x=\"%.1f\" y=\"%.1f\" font-size=\"%zu\" text-anchor=\"bottom-left\" fill=\"black\">%s</text>\n",
		x, y,
		svgSettings.pRadius,
		(uint16_t)color.r, (uint16_t)color.g, (uint16_t)color.b,
		x + svgSettings.pRadius, y - svgSettings.pRadius,
		svgSettings.fontSize,
		p->id.str
	) > 0;
}
bool svg_text(FILE * restrict fp, float x, float y, const char * restrict str)
{
	assert(fp  != NULL);
	assert(str != NULL);

	const double dx = (double)x * svgSettings.scalex, dy = -(double)y * svgSettings.scaley;

	return fprintf(
		fp,
		"<text x=\"%.1f\" y=\"%.1f\" font-size=\"%zu\" text-anchor=\"middle\" fill=\"black\">%s</text>\n",
		dx, dy - 10.0,
		svgSettings.fontSize,
		str
	) > 0;
}

bool svg_footer(FILE * restrict fp)
{
	assert(fp != NULL);
	return fputs("\n</svg>\n", fp) > 0;
}
