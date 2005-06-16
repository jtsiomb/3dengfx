
inline scalar_t lerp(scalar_t a, scalar_t b, scalar_t t) {
	return a + (b - a) * t;
}

inline scalar_t bspline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t) {
	return bspline(Vector4(a, b, c, d), t);
}

inline scalar_t catmull_rom_spline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t) {
	return catmull_rom_spline(Vector4(a, b, c, d), t);
}
