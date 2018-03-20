typedef unsigned long TypeQual;

extern int sprintf();

int sc_name(char array[], TypeQual tq)
{
    switch((( tq ) & (      0x01  |     0x02  |   0x04  |     0x08  |    0x10  |       0x20 ) ) ) {
      case    0x10 :
	sprintf(array, "typedef");
	return 7;
      case     0x02 :
	sprintf(array, "extern");
	return 6;
      case     0x08 :
	sprintf(array, "static");
	return 6;
      case       0x01 :
	sprintf(array, "auto");
	return 4;
      case   0x04 :
	sprintf(array, "register");
	return 8;
      case       0x20 :
	 
	return 0;
      case 0:  
	array[0] = 0;
	return 0;
      default:  
	sprintf(array, "SC_ERROR=%04x", (int)tq);
	return 13;
    }
}
