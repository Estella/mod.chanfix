DELETE FROM languages;
DELETE FROM help;

COPY "languages" FROM stdin;
1	EN	English	31337	0
\.

COPY "help" FROM stdin;
QUOTE	1	This command should never be used.\nUsing it would be bad.\nHow bad?\nTry to imagine all life stopping instantaniously, and every molecule in your body exploding at the speed of light.
\.

