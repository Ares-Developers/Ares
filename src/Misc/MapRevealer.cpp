#include "MapRevealer.h"

#include <MouseClass.h>

MapRevealer::MapRevealer(const CoordStruct& coords) :
	BaseCell(this->TranslateBaseCell(coords)),
	RequiredChecks(RequiresExtraChecks())
{
	auto const& Rect = MapClass::Instance->MapRect;
	this->MapWidth = Rect.Width;
	this->MapHeight = Rect.Height;

	this->CheckedCells[0] = {7, static_cast<short>(this->MapWidth + 5)};
	this->CheckedCells[1] = {13, static_cast<short>(this->MapWidth + 11)};
	this->CheckedCells[2] = {static_cast<short>(this->MapHeight + 13),
		static_cast<short>(this->MapHeight + this->MapWidth - 15)};
}

MapRevealer::MapRevealer(const CellStruct& cell)
	: MapRevealer(MapClass::Instance->GetCellAt(cell)->GetCoordsWithBridge())
{ }

void MapRevealer::Process1(CellClass* const pCell, bool fog, bool add) const {
	pCell->Flags &= ~0x40;

	if(fog) {
		if((pCell->Flags & 3) != 3 && pCell->CopyFlags & cf2_NoShadow) {
			MouseClass::Instance->vt_entry_98(pCell->MapCoords, HouseClass::Player);
		}
	} else {
		if(this->IsCellAllowed(pCell->MapCoords)) {
			MouseClass::Instance->vt_entry_94(pCell->MapCoords, HouseClass::Player, add);
		}
	}
}
