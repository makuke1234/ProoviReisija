#include "svgWriter.h"

struct svgSettings
{
	size_t pRadius, fontSize;

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
		.r = (color >> 24) & 0xFF,
		.g = (color >> 16) & 0xFF,
		.b = (color >>  8) & 0xFF,
		.a =  color        & 0xFF
	};
}


void svg_init()
{
	svgSettings = (struct svgSettings){
		.pRadius = 2,
		.fontSize = 20
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

bool svg_header(FILE * restrict fp, size_t width, size_t height, svgRGB_t backColor)
{
	return fprintf(
		fp,
		"<svg version=\"1.1\" width=\"%zu\" height=\"%zu\" xmlns=\"http://www.w3.org/2000/svg\">\n"
		"<rect width=\"100%%\" height=\"100%%\" fill=\"rgb(%hu, %hu, %hu)\" />\n\n",
		width,
		height,
		(uint16_t)backColor.r, (uint16_t)backColor.g, (uint16_t)backColor.b
	) > 0;
}

bool svg_line(FILE * restrict fp, const line_t * restrict l, svgRGB_t color)
{
	return fprintf(
		fp,
		"<"
	);
}
bool svg_linePoint(FILE * restrict fp, const line_t * restrict l, svgRGB_t color, bool drawSrc)
{
	return svg_line(fp, l, color) && ((drawSrc && svg_point(fp, l->src, color)) || !drawSrc) && svg_point(fp, l->dst, color);
}
bool svg_point(FILE * restrict fp, const point_t * restrict p, svgRGB_t color)
{
	return fprintf(
		fp,
		"<circle cx=\"%zu\" cy=\"%zu\" r=\"%zu\" fill=\"rgb(%hu, %hu, %hu)\" />\n"
		"<text cx=\"%zu\" cy=\"%zu\" font-size=\"%zu\" text-anchor=\"\" fill=\"black\">%s</text>\n",
		p->x, p->y,
		svgSettings.pRadius,
		(uint16_t)color.r, (uint16_t)color.g, (uint16_t)color.b,
		p->x, p->y,
		svgSettings.fontSize,
		p->id.str
	);
}

bool svg_footer(FILE * restrict fp)
{
	return fputs("\n</svg>\n", fp) > 0;
}
