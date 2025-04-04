static int is_inside(double px, double py, double x, double y, double w, double h)
{
	return (x <= px && px <= x + w) && (y <= py && py <= y + h);
}
