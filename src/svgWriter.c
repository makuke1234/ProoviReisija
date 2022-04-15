#include "svgWriter.h"

struct svgSettings
{
	size_t pRadius, fontSize;

	double scalex, scaley;
	const char * font;

	const char * fill;
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
		.fontSize = SVG_FONT_SIZE,
		.font     = "",
		.fill     = SVG_FILL
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
void svg_setFont(const char * restrict font)
{
	svgSettings.font = font;
}
void svg_setTextFill(const char * restrict fill)
{
	svgSettings.fill = fill;
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
		"<svg version=\"1.1\" width=\"%zu\" height=\"%zu\" viewBox=\"%lld %lld %zu %zu\" font-family=\"%s\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<rect x=\"%lld\" y=\"%lld\" width=\"100%%\" height=\"100%%\" fill=\"rgb(%hu, %hu, %hu)\" />\n\n",
		(size_t)SVG_WIDTH, newHeight,
		vx, -vy, (size_t)SVG_WIDTH, newHeight,
		svgSettings.font,
		vx, -vy,
		(uint16_t)backColor.r, (uint16_t)backColor.g, (uint16_t)backColor.b
	) >= 0;
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
		(size_t)SVG_LINE_STROKE
	) >= 0;
}
bool svg_linePoint(FILE * restrict fp, const line_t * restrict l, svgRGB_t color, bool drawSrc, bool drawText)
{
	assert(fp != NULL);
	assert(l  != NULL);
	assert(l->src != NULL);
	assert(l->dst != NULL);

	return svg_line(fp, l, color) && ((drawSrc && svg_point(fp, l->src, color, drawText)) || !drawSrc) && svg_point(fp, l->dst, color, drawText);
}
bool svg_point(FILE * restrict fp, const point_t * restrict p, svgRGB_t color, bool drawText)
{
	assert(fp != NULL);
	assert(p  != NULL);
	assert(p->id.str != NULL);

	const double x = (double)p->x * svgSettings.scalex, y = -(double)p->y * svgSettings.scaley;
	
	return drawText ? fprintf(
		fp,
		"<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%zu\" fill=\"rgb(%hu, %hu, %hu)\" />\n"
		"<text x=\"%.1f\" y=\"%.1f\" font-size=\"%zu\" text-anchor=\"start\" fill=\"black\">%s</text>\n",
		x, y,
		svgSettings.pRadius,
		(uint16_t)color.r, (uint16_t)color.g, (uint16_t)color.b,
		x + (double)svgSettings.pRadius, y - (double)svgSettings.pRadius,
		svgSettings.fontSize,
		p->id.str
	) >= 0 : fprintf(
		fp,
		"<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%zu\" fill=\"rgb(%hu, %hu, %hu)\" />\n",
		x, y,
		svgSettings.pRadius,
		(uint16_t)color.r, (uint16_t)color.g, (uint16_t)color.b
	) >= 0;
}

static const char * dombases[svgBase_size] = {
	"auto",
	"use-script",
	"no-change",
	"reset-size",
	"ideographic",
	"alphabetic",
	"hanging",
	"mathematical",
	"central",
	"middle",
	"text-after-edge",
	"text-before-edge",
	"inherit"
};
static const char * aligns[svgAlign_size] = {
	"start",
	"middle",
	"end"
};
bool svg_text(FILE * restrict fp, float x, float y, const char * restrict str, svgBase_t baseline, svgAlign_t align)
{
	assert(fp  != NULL);
	assert(str != NULL);
	assert(baseline >= 0 && align >= 0);
	assert(baseline < svgBase_size && align < svgAlign_size);

	const double dx = (double)x * svgSettings.scalex, dy = -(double)y * svgSettings.scaley;

	return fprintf(
		fp,
		"<text x=\"%.1f\" y=\"%.1f\" font-size=\"%zu\" dominant-baseline=\"%s\" text-anchor=\"%s\" fill=\"%s\">%s</text>\n",
		dx, dy,
		svgSettings.fontSize,
		dombases[baseline], aligns[align], svgSettings.fill,
		str
	) >= 0;
}
bool svg_textRot(FILE * restrict fp, float x, float y, const char * restrict str, svgBase_t baseline, svgAlign_t align, float rotation)
{
	assert(fp  != NULL);
	assert(str != NULL);
	assert(baseline >= 0 && align >= 0);
	assert(baseline < svgBase_size && align < svgAlign_size);

	const double dx = (double)x * svgSettings.scalex, dy = -(double)y * svgSettings.scaley;

	return fprintf(
		fp,
		"<text font-size=\"%zu\" dominant-baseline=\"%s\" text-anchor=\"%s\" fill=\"%s\" transform=\"translate(%.1f, %.1f) rotate(%.6g)\">%s</text>\n",
		svgSettings.fontSize,
		dombases[baseline], aligns[align], svgSettings.fill,
		dx, dy,
		(double)rotation,
		str
	) >= 0;
}

bool svg_footer(FILE * restrict fp)
{
	assert(fp != NULL);
	return fputs("\n</svg>\n", fp) >= 0;
}
