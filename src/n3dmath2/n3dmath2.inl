
inline scalar_t Lerp(scalar_t a, scalar_t b, scalar_t t) {
	return a + (b - a) * t;
}

inline scalar_t BSpline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t) {
	return BSpline(Vector4(a, b, c, d), t);
}

inline scalar_t CatmullRomSpline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t) {
	return CatmullRomSpline(Vector4(a, b, c, d), t);
}
