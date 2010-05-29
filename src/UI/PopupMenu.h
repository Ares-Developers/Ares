#ifndef ARES_POPUPMENU_H_
#define ARES_POPUPMENU_H_

#include <GeneralStructures.h>

class PopupMenu {
public:
	static Point2D PixelCoords;
	static CellStruct CellCoords;
	static CoordStruct LeptonCoords;

	static RectangleStruct MenuBox;
	static RectangleStruct MenuItem;

	static short HighlightedItem;
	static bool Drawing;
};
#endif
