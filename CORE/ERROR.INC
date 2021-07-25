{*--------------------------------------------------------------------------*}
{*                             E R R O R. I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Error handling for the core game engine                     *}
{* Prequisites: None                                                        *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_ERROR__}
{$define __CORE_ERROR__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	ERR_MAX_LENGTH	= 128;

var	ERR_buffer	: string[ERR_MAX_LENGTH];
	ERR_occured	: boolean;

procedure	ERR_init;                               forward;
procedure       ERR_throw_no_param(msg: string);        forward;
procedure	ERR_throw(msg, param: string);          forward;
function	ERR_get: string;                        forward;
procedure	ERR_die(msg, text: string);             forward;

procedure	THROW_MALLOC_ERR;                       forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

procedure	ERR_init;
begin
	ERR_occured := FALSE;
        ERR_buffer  := '';
end;

procedure       ERR_throw_no_param(msg: string);
begin
        ERR_occured := TRUE;
        ERR_buffer  := msg;
end;

procedure	ERR_throw(msg, param: string);
begin
        ERR_occured := TRUE;
        ERR_buffer  := msg + param;
end;

function	ERR_get: string;
begin
	ERR_get := ERR_buffer;
end;

procedure	THROW_MALLOC_ERR;
begin
	ERR_throw_no_param('Memory allocation error.');
end;

procedure	ERR_die(msg, text: string);
begin
        writeln(msg, text);
        halt;
end;

{$endif}