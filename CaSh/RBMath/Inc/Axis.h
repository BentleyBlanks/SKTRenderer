#pragma once

enum Axiss
{
	None,
	X,
	Y,
	Z,
	XY = X|Y,
	XZ = X|Z,
	YZ = Y|Z,
	XYZ = X|Y|Z,
	ALL = XYZ,

	ZROTATION = YZ,



};