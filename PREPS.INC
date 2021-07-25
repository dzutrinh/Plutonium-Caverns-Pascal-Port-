{*---------- D E F A U L T  C O M P I L I N G  D I R E C T I V E S ---------*}

{$ifdef __TMT__}		{ TMT Pascal directives }

	{$MMX+,OPTREG+,AMD+,OA-,W-,A+,B-,Q-,V+,X+,T-,P+,R-}

{$else}
{$ifdef VER70}			{ Turbo Pascal directives }
	{$ifdef __DEBUG__}	{ debug  : turn on all checks }

		{$A+,B-,D+,L+,S+,R+,Q+,X+,O-,P+,T-,V+,F+,I+,E+,N-}

	{$else}			{ release: turn off all checks }

		{$A+,B-,D+,F+,I-,L+,O-,P+,Q-,R-,S-,T-,V+,X+,E-,N+}

	{$endif}
	{$M 65520, 0, 655360}	{ request as much memory as possible }
{$else}

        'TMT Pascal 3.9 or Turbo Pascal 7.0 is required'

{$endif}
{$endif}