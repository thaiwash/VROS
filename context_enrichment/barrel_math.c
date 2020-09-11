function barrel() { /* Lens Barrel Distionion Correction */
	/* Radial Polynomial Distortion (de-normalized)
	 * 
	 * 
	 * Convert input arguments into mapping coefficients, this this case
     * we are mapping (distorting) colors, rather than coordinates.
      coeff = GenerateCoefficients(image, &distort_method, number_arguments,
                arguments, number_colors, exception);
	 * 
		double *coeff;
		coeff=(double *) RelinquishMagickMemory(coeff);
		
	 * 
	 *  */
	double r,fx,fy,gx,gy;
	d.x -= coeff[8];
	d.y -= coeff[9];
	r = sqrt(d.x*d.x+d.y*d.y);
	if ( r > MagickEpsilon ) {
	  fx = ((coeff[0]*r + coeff[1])*r + coeff[2])*r + coeff[3];
	  fy = ((coeff[4]*r + coeff[5])*r + coeff[6])*r + coeff[7];
	  gx = ((3*coeff[0]*r + 2*coeff[1])*r + coeff[2])/r;
	  gy = ((3*coeff[4]*r + 2*coeff[5])*r + coeff[6])/r;
	  /* adjust functions and scaling for 'inverse' form */
	  if ( method == BarrelInverseDistortion ) {
		fx = 1/fx;  fy = 1/fy;
		gx *= -fx*fx;  gy *= -fy*fy;
	  }
	  /* Set the source pixel to lookup and EWA derivative vectors */
	  s.x = d.x*fx + coeff[8];
	  s.y = d.y*fy + coeff[9];
	  ScaleFilter( resample_filter[id],
		  gx*d.x*d.x + fx, gx*d.x*d.y,
		  gy*d.x*d.y,      gy*d.y*d.y + fy );
	}
	else {
	  /* Special handling to avoid divide by zero when r==0
	  **
	  ** The source and destination pixels match in this case
	  ** which was set at the top of the loop using  s = d;
	  ** otherwise...   s.x=coeff[8]; s.y=coeff[9];
	  */
	  if ( method == BarrelDistortion )
		ScaleFilter( resample_filter[id],
			 coeff[3], 0, 0, coeff[7] );
	  else /* method == BarrelInverseDistortion */
		/* FUTURE, trap for D==0 causing division by zero */
		ScaleFilter( resample_filter[id],
			 1.0/coeff[3], 0, 0, 1.0/coeff[7] );
	}
	break;
  }
 }
