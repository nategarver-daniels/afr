#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ASPHeader.h"
#include "fitsio.h"

int ReadPSRFITSHdr(struct ASPHdr *hdr, fitsfile *Fin)
{
  int     i, knul=0, i_param, n_param, anynull, retval;
  int     hdutype, colnum, anynul, status=0, tempint;
  double  dnul=0.0, tempdouble;
  float   fnul=0;
  long    nrows=0;
  /* tempstr1 is for reading PSRPARAM table -- has width 128 */
  char    tempRA[32], tempDec[32], telname[6], tempstr1[128], tempstr2[256];
  double  hr, deg, min, sec, sgn;

  retval = -1;

  /* Move to primary header */
  if (fits_movabs_hdu(Fin, 1, &hdutype, &status)) {
    fprintf(stderr, "ERROR ReadPSRFITSHdr: Cannot move to primary HDU\n");
    return -1;
  }

/* Header version no. */
  fits_read_key(Fin, TSTRING, "HDRVER", hdr->gen.HdrVer, NULL, &status); 
  status=0; 
/* Header FITS definition type */
  fits_read_key(Fin, TSTRING, "FITSTYPE", hdr->gen.FitsType, NULL, &status); 
  status=0;
/****** DATE GOES HERE ******/ 

/* Observer name */
  fits_read_key(Fin, TSTRING, "OBSERVER", hdr->gen.Observer, NULL, &status); 
  status=0; 
/* Project ID */
  fits_read_key(Fin, TSTRING, "PROJID", hdr->gen.ProjID, NULL, &status); 
  status=0; 
/* Observatory code */
  fits_read_key(Fin, TSTRING, "TELESCOP", hdr->obs.ObsvtyCode, NULL, &status); 
  status=0; 
  /*  if(!strncmp(telname,"GBT",3))
      sprintf(hdr->obs.ObsvtyCode, "1"); */

/****** ANT_X, ANT_Y, and ANT_Z goes here ******/

/* Frontend name GREG/CH etc */
  fits_read_key(Fin, TSTRING, "FRONTEND", hdr->gen.FEName, NULL, &status); 
  status=0; 

/* Number of receiver polarisation channels */
  //fits_read_key(Fin, TSTRING, "NRCVR", hdr->gen.NRcvr, NULL, &status); status=0;

/* Polarisation type CIRC/LIN */
  fits_read_key(Fin, TSTRING, "FD_POLN", hdr->gen.FEPol, NULL, &status); 
  status=0;
  if (!strncmp(hdr->gen.FEPol, "LIN", 3))
    strcpy(hdr->gen.FEPol, "L");
  else if (!strncmp(hdr->gen.FEPol, "CIRC", 4))
    strcpy(hdr->gen.FEPol, "C");
  else {
    fprintf(stderr,"ReadPSRFITSHdr ERROR: Unrecognized polarization basis %s\n",
	    hdr->gen.FEPol);
    return -1;
  }
  

/****** FD_HAND, FD_SANG, FD_XYPH Here ******/

/* Backend name ASP/ABPP/WAPP etc */
  fits_read_key(Fin, TSTRING, "BACKEND", hdr->gen.BEName, NULL, &status); 
  status=0; 
/* Backend Config filename */
  fits_read_key(Fin, TSTRING, "BECONFIG", hdr->gen.BEConfFile, NULL, &status); 
  status=0; 
/****** BE_PHASE, BE_DCC, BE_DELAY ******/
/****** TCYCLE ******/

/* Observation mode PSR/CAL/SPECTRAL etc */
  fits_read_key(Fin, TSTRING, "OBS_MODE", hdr->gen.ObsMode, NULL, &status); 
  status=0; 

/****** DATE-OBS ******/

/* Overall Central Sky Frequency [MHz] */
  fits_read_key(Fin, TDOUBLE, "OBSFREQ", &hdr->obs.FSkyCent, NULL, &status); 
  status=0; 
/* Bandwidth for observation [MHz] */
  fits_read_key(Fin, TDOUBLE, "OBSBW", &hdr->obs.BW, NULL, &status); 
  status=0; 
  /* Ensure BW is a positive number -- will be calculating 
     sideband later anyway*/
  hdr->obs.BW = fabs(hdr->obs.BW);
/* Number of frequency channels (original number) */
  fits_read_key(Fin, TINT, "OBSNCHAN", &hdr->obs.NChanOrig, NULL, &status); 
  status=0; 

/****** CHAN_DM ******/

/* Source Name */
  fits_read_key(Fin, TSTRING, "SRC_NAME", hdr->target.PSRName, NULL, &status); 
  status=0; 
/* Coordinate mode for following 4 entries  J2000/GAL/ECLIPTIC/AZEL etc. */
  fits_read_key(Fin, TSTRING, "COORD_MD", hdr->target.CoordMode, NULL, &status);
  status=0;

  /****** EQUINOX ******/
/* Epoch of the coordinates */
  /* Read in as string and convert depending on PSRFITS version */
  fits_read_key(Fin, TFLOAT, "EQUINOX", &hdr->target.Epoch, NULL, &status); 
  status=0; 
 
/* coordinate 1 at start time */
  fits_read_key(Fin, TSTRING, "STT_CRD1",hdr->target.StartCrd1, NULL, &status);
  status=0; 
/* coordinate 2 at start time */
  fits_read_key(Fin, TSTRING, "STT_CRD2",hdr->target.StartCrd2, NULL, &status);
  status=0; 
/* coordinate 1 at stop time */
  fits_read_key(Fin, TSTRING, "STP_CRD1", hdr->target.StopCrd1, NULL, &status);
  status=0; 
/* coordinate 2 at stop time */
  fits_read_key(Fin, TSTRING, "STP_CRD2", hdr->target.StopCrd2, NULL, &status);
  status=0; 

/* Right Assention hh.hhhhhhhhhhhhhh */
  fits_read_key(Fin, TSTRING, "RA", tempRA, NULL, &status); 
  if (status != 0)   /* if no RA keyword, copy in STT_CRD1 keyword */
    strcpy(tempRA, hdr->target.StartCrd1); 
  sscanf(tempRA, "%lf:%lf:%lf", &hr, &min, &sec);
  hdr->target.RA = hr + min/60. + sec/3600.;
/* Declination dd.dddddddddddddd */
  fits_read_key(Fin, TSTRING, "DEC", tempDec, NULL, &status); 
  if (status != 0)   /* if no DEC keyword, copy in STT_CRD2 keyword */
    strcpy(tempDec, hdr->target.StartCrd2);
  sscanf(tempDec, "%lf:%lf:%lf", &deg, &min, &sec);
  if(deg > 0.) sgn = 1.; else sgn = -1;
  hdr->target.Dec = deg + sgn*(min/60. + sec/3600.);
  
/* Track mode TRACK/SLEW/DRIFT etc. */
  fits_read_key(Fin, TSTRING, "TRK_MODE", hdr->target.TrackMode, NULL, &status);
  status=0; 
/* Requested observation length (s) */
  fits_read_key(Fin, TDOUBLE, "SCANLEN", &hdr->obs.ObsLength, NULL, &status); 
  status=0; 
/* Integer MJD of starting sample */
  fits_read_key(Fin, TINT,    "STT_IMJD", &hdr->obs.IMJDStart, NULL, &status); 
  status=0;
/* Start Time (INT) second  past 0.0h */
  fits_read_key(Fin, TINT, "STT_SMJD", &hdr->obs.StartTime, NULL, &status); 
  status=0; 
 
/* Clock offset (sec) */
  fits_read_key(Fin, TDOUBLE, "STT_OFFS", &hdr->obs.ClockOffset, NULL, &status);
  status=0; 
/* Start LST */
  fits_read_key(Fin, TDOUBLE, "STT_LST", &hdr->target.StartLST, NULL, &status); 
  status=0; 

  /* Don't bother to get this DM/RM info if this is a cal file */
  if (strncmp(hdr->gen.ObsMode, "CAL", 3)) {
      
      /* If this is GUPPI data, we need to read in DM/RM from the ephemeris table */
    //    if (!strncmp(hdr->gen.BEName, "GUPPI", 5)){
      /* Go to PSREPHEM table -- if nonexistent, see if there is a PSRPARAM
	 table to extract DM */
      
      if(fits_movnam_hdu(Fin, BINARY_TBL, "PSREPHEM", 0, &status)) {
	/* Now try PSRPARAM table */
	status=0;
	if(fits_movnam_hdu(Fin, BINARY_TBL, "PSRPARAM", 0, &status)) {
	  fprintf(stderr, "ERROR ReadASPHdr: neither PSREPHEM nor PSRPARAM tables exist!\n");
	  return -1;
	}
	else {
	  /* Need to read row by row until DM is seen -- and 
	     make sure it's not DM1, etc.*/
	  fits_get_num_rows (Fin, &nrows, &status);
	  if(!nrows){
	    fprintf(stderr, "ERROR ReadPSRFITSHdr: No rows in PSRPARAM table.\n");
	    return -1;
	  }

	  /* First get number of rows */
	  fits_read_key(Fin, TINT, "NAXIS2", &n_param, NULL, &status); 
	  if(nrows!=n_param){
	    fprintf(stderr, "ERROR ReadPSRFITSHdr: Problem with row numbers in PSRPARAM table.\n");
	    return -1;
	  }
	  /*  will need to read in DM and RM from here as a table column... */
	  if(fits_get_colnum(Fin, CASEINSEN, "PARAM", &colnum, &status)){
	    fprintf(stderr, "ERROR ReadPSRFITSHdr: Problem reading PSRPARAM table.\n");
	    return -1;
	  }

	  char* temp = &(tempstr1[0]);
	  for (i_param=1; i_param <= n_param; i_param++) {
	    //#if 0
	    if(fits_read_col(Fin, TSTRING, colnum, i_param, 1, 1, 
			     0, &temp, &anynul, &status)) {
	      fprintf(stderr, "ERROR ReadPSRFITSHdr: Problem reading PSRPARAM table.\n");
	      return -1;
	    }
	    //#endif


	    printf("OH HAI again 2\n");
	    /* Parse string */
	    printf("%s\n",tempstr1);
	    if(!strncmp(&tempstr1[0], "DM ", 3)){
	      sscanf(tempstr1, "%s %lfD%3d", tempstr2, &tempdouble, &tempint);
	      sprintf(tempstr2,"%lfE%d \0", tempdouble, tempint);
	      sscanf(tempstr2, "%lf", &hdr->obs.DM);
	      /* Need to set RM to zero in this case */
	      hdr->obs.RM = 0.;
	      break;
	    }
	    if(i_param==n_param-1){
	      fprintf(stderr, "ERROR ReadPSRFITSHdr:  Could not find DM in PSRPARAM table.\n");
	      return -1;
	    }	    
	  }

	}
      }
      else{
	
	/*  will need to read in DM and RM from here as a table column... */
	if(fits_get_colnum(Fin, CASEINSEN, "DM", &colnum, &status)){
	  fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read DM column.\n");
	  return -1;
	}
	if(fits_read_col(Fin, TDOUBLE, colnum, 1, 1, 1, NULL, &hdr->obs.DM, &anynul, &status)) {
	  fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read DM value.\n");
	}
	
	if(fits_get_colnum(Fin, CASEINSEN, "RM", &colnum, &status)){
	  fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read RM column.\n");
	  return -1;
	}
	if(fits_read_col(Fin, TDOUBLE, colnum, 1, 1, 1, NULL, &hdr->obs.RM, &anynul, &status)) {
	  fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read RM value.\n");
	}
      }
      // }
  }
  

  /* Dispersion Measure */
    //  fits_read_key(Fin, TDOUBLE, "DM", &hdr->obs.DM, NULL, &status); status=0; 
  /* Rotation Measure */
  //fits_read_key(Fin, TDOUBLE, "RM", &hdr->obs.RM, NULL, &status); status=0; 

  /* Will need to get data info from different headers and tables,
     depending on version of PSRFITS... */

  if (!strncmp(hdr->gen.HdrVer, "1.26", 4)) {

    /* Go to HISTORY table */
    if(fits_movnam_hdu(Fin, BINARY_TBL, "HISTORY", 0, &status)) {
      fprintf(stderr, "ERROR ReadASPHdr: HISTORY table does not exist!\n");
      return -1;
    } 
    
   /* Number of profile bins */
    if(fits_get_colnum(Fin, CASEINSEN, "NBIN", &colnum, &status)){
      fprintf(stderr,"ERROR ReadPSRFITSHdr: Could not read NBIN column.\n");
      return -1;
    }
    if(fits_read_col(Fin, TINT, colnum, 1, 1, 1, NULL, 
		     &hdr->redn.RNBinTimeDump, &anynul, &status)) {
      fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read NBIN value.\n");
    }
   
    /* Number of frequency channels (current file) */
    if(fits_get_colnum(Fin, CASEINSEN, "NCHAN", &colnum, &status)){
      fprintf(stderr,"ERROR ReadPSRFITSHdr: Could not read NCHAN column.\n");
      return -1;
    }
    if(fits_read_col(Fin, TINT, colnum, 1, 1, 1, NULL, 
		     &hdr->obs.NChan, &anynul, &status)) {
      fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read NCHAN value.\n");
      return -1;
    }

   /* Channel/sub-band width (current file) */
    if(fits_get_colnum(Fin, CASEINSEN, "CHAN_BW", &colnum, &status)){
      fprintf(stderr,"ERROR ReadPSRFITSHdr: Could not read CHAN_BW column.\n");
      return -1;
    }
    if(fits_read_col(Fin, TDOUBLE, colnum, 1, 1, 1, NULL, 
		     &hdr->obs.ChanWidth, &anynul, &status)) {
      fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read CHAN_BW value.\n");
      return -1;
    }
    /* Ensure ChanWidth is positive -- will calculate sideband later anyway */
    hdr->obs.ChanWidth = fabs(hdr->obs.ChanWidth);

    /* Number of polarizations */
    if(fits_get_colnum(Fin, CASEINSEN, "NPOL", &colnum, &status)){
      fprintf(stderr,"ERROR ReadPSRFITSHdr: Could not read NPOL column.\n");
      return -1;
    }
    if(fits_read_col(Fin, TINT, colnum, 1, 1, 1, NULL, 
		     &hdr->obs.NPoln, &anynul, &status)) {
      fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read NPOL value.\n");
      return -1;
    }

    /* Polarization identifier */
    if(fits_get_colnum(Fin, CASEINSEN, "POL_TYPE", &colnum, &status)){
      fprintf(stderr,"ERROR ReadPSRFITSHdr: Could not read POL_TYPE column.\n");
      return -1;
    }
    int i_char;
    char* temppoltype = &(hdr->obs.PolnType[0]);
    fits_read_col(Fin, TSTRING, colnum, 1, 1, 1, NULL, 
		  &temppoltype, &anynul, &status);
    if(status!=0){
      fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read POL_TYPE value.\n");
      fits_report_error(stderr, status);
      return -1;
    }
    
    /* Go to SUBINT table */
    if(fits_movnam_hdu(Fin, BINARY_TBL, "SUBINT", 0, &status)) {
      fprintf(stderr, "ERROR ReadASPHdr: SUBINT table does not exist!\n");
      return -1;
    } 
  }
  else{

  /* Go to SUBINT table */
    if(fits_movnam_hdu(Fin, BINARY_TBL, "SUBINT", 0, &status)) {
      fprintf(stderr, "ERROR ReadASPHdr: SUBINT table does not exist!\n");
      return -1;
    } 
    /* First, if not GUPPI data, read in DM and RM from this table's header */
    if(strncmp(hdr->gen.BEName, "GUPPI", 5)) {
      fits_read_key(Fin, TDOUBLE, "DM", &hdr->obs.DM, NULL, &status); 
      status=0; 
      /* Rotation Measure */
      fits_read_key(Fin, TDOUBLE, "RM", &hdr->obs.RM, NULL, &status); 
      status=0; 
    }
    
    /***** TBIN -- time per bin or sample *****/
    
    /* Number of profile bins */
    fits_read_key(Fin, TINT, "NBIN", &hdr->redn.RNBinTimeDump, NULL, &status);
    status=0;
    /* Number of frequency channels (current file) */
    fits_read_key(Fin, TINT, "NCHAN", &hdr->obs.NChan, NULL, &status); 
    status=0; 
    /* Channel/sub-band width (current file) */
    fits_read_key(Fin, TDOUBLE, "CHAN_BW", &hdr->obs.ChanWidth, NULL, &status);
    status=0;
    /* Ensure ChanWidth is positive -- will calculate sideband later anyway */
    hdr->obs.ChanWidth = fabs(hdr->obs.ChanWidth);
    /* Number of polarizations */
    fits_read_key(Fin, TINT, "NPOL", &hdr->obs.NPoln, NULL, &status); 
    status=0; 
    /* Polarization identifier */
    fits_read_key(Fin, TSTRING, "POL_TYPE", hdr->obs.PolnType, NULL, &status); 
    status=0; 
    /* Read in channel values */
    /* NOTE that there is a separate DAT_FREQ table for each row...  
       hopefully will be the same for all integrations!  
       Will put in a safeguard at some point */
    
  }
  
  /* Number of integrations */
  fits_read_key(Fin, TINT, "NAXIS2", &hdr->redn.RNTimeDumps, NULL, &status); 
  status=0;
  
  if(fits_get_colnum(Fin, CASEINSEN, "DAT_FREQ", &colnum, &status)){
    fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read DAT_FREQ column.\n");
    return -1;
  }
  if(fits_read_col(Fin, TDOUBLE, colnum, 1, 1, hdr->obs.NChan, NULL, &hdr->obs.ChanFreq[0], &anynul, &status)) {
    fprintf(stderr, "ERROR ReadPSRFITSHdr: Could not read DAT_FREQ value.\n");
    return -1;
  }
  
  /* Set BW -- should work for all backend versions of PSRFITS */
  hdr->obs.BW = hdr->obs.ChanWidth * ((double)hdr->obs.NChan);
  /* Set Centre Frequency here -- should work for all versions of PSRFITS */
  hdr->obs.FSkyCent = 0.5*(hdr->obs.ChanFreq[0] + hdr->obs.ChanFreq[hdr->obs.NChan-1]);



#if 0

  /* Little hard-coded exception for Nancay data taken before MJD 53686.05 
     -- Those channels are labelled 1 MHz too high */
  if (!strcmp(hdr->obs.ObsvtyCode, "f") && 
     ((double)hdr->obs.IMJDStart + ((double)hdr->obs.StartTime/86400.0)) 
     <= 53686.05) {
    printf("Adjusting Nancay data frequency labels by -1.0 MHz...\n");
    for (i=0; i<hdr->obs.NChan; i++) {
      if (hdr->obs.ChanFreq[i] >= 1350.0 && hdr->obs.ChanFreq[i] <= 1450.0) 
	hdr->obs.ChanFreq[i] -= 1.0;
    }
  }

#endif

  retval = 0;

  return retval;
}
