

inline scalar_t Cerp(scalar_t x0 , scalar_t x1 ,
		     scalar_t x2 , scalar_t x3 ,
		     scalar_t t)
{
	scalar_t a0,a1,a2,a3,t2;

	t2 = t*t;
	a0 = x3 - x2 - x0 + x1;
	a1 = x0 - x1 - a0;
	a2 = x2 - x0;
	a3 = x1;

	return(a0*t*t2+a1*t2+a2*t+a3);
}

inline int ClampInteger(int i,int from,int to)
{
	int r=i;
	if (r<from) r=from;
	if (r>to) r =to;

	return r;
}


