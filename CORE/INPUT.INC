{*--------------------------------------------------------------------------*}
{*                            I N P U T . I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Keyboard driver for the core game engine                    *}
{* Prequisites: DOS unit (getintvec/setintvec)                              *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_INPUT__}
{$define __CORE_INPUT__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	ARROW_RIGHT	= 0;
	ARROW_UP	= 1;
	ARROW_LEFT	= 2;
	ARROW_DOWN	= 3;

        KEY_Z		= $2C;
        KEY_R		= $13;
        KEY_ESC		= $01;
        KEY_ENTER	= $1C;

        KBD_UP		= 0;
	KBD_DOWN	= 1;
	KBD_PRESSED	= 2;
	KBD_RELEASED	= 3;

	KBD_BUFFER_SIZE	= $60;
	KBD_MAX_BUTTONS	= 8;

{ INPUT API }
procedure       KBD_init;                                   forward;
procedure	KBD_destroy;                                forward;
function	KBD_get_key(id: int16): int16;              forward;
function	KBD_get_arrow_key(id: int16): int16;        forward;
procedure	KBD_add_button(index, key: int16);          forward;
function	KBD_get_button(id: int16): int16;           forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

type    KBD_BYTE_ARR	= array[0..KBD_BUFFER_SIZE-1] of uint8;
        KBD_BOOL_ARR	= array[0..KBD_BUFFER_SIZE-1] of boolean;

var	KBD_nrm_keys	: KBD_BYTE_ARR;
        KBD_ext_keys	: KBD_BYTE_ARR;
	KBD_old_nrm	: KBD_BYTE_ARR;
        KBD_old_ext	: KBD_BYTE_ARR;
        KBD_nrm_read	: KBD_BOOL_ARR;
        KBD_ext_read	: KBD_BOOL_ARR;
        KBD_buttons	: array[0..KBD_MAX_BUTTONS-1] of int16;
{$ifdef	__TMT__}
	KBD_old_handler	: farpointer;
{$else}
	KBD_old_handler : pointer;
{$endif}
	KBD_buffer	: uint8;

const	KBD_ARROW_CODES	: array[0..3] of int16 = (77, 72, 75, 80);

procedure       KBD_handler; interrupt;
var	rawcode	: uint8;
        makebrk	: uint8;
	scan	: int16;
begin
        rawcode := port[$60];
        makebrk := uint8((rawcode and $80) = 0);
        scan	:= rawcode and $7F;

        if KBD_buffer = $E0 then
        begin
        	if scan < $60 then
                begin
                        KBD_old_ext[scan]  := KBD_ext_keys[scan];
                        KBD_ext_keys[scan] := makebrk;
                        if KBD_ext_keys[scan] <> KBD_old_ext[scan] then
                                KBD_ext_read[scan] := FALSE;
                end;
                KBD_buffer := 0;
        end else
	if (KBD_buffer >= $E1) and (KBD_buffer <= $E2) then
		KBD_buffer := 0 else
        if (rawcode >= $E0) and (rawcode <= $E2) then
		KBD_buffer := rawcode else
        if scan < $60 then
        begin
        	KBD_old_nrm[scan]  := KBD_nrm_keys[scan];
                KBD_nrm_keys[scan] := makebrk;
                if KBD_nrm_keys[scan] <> KBD_old_nrm[scan] then
			KBD_nrm_read[scan] := FALSE;
        end;
	port[$20] := $20;
end;

function	KBD_get_value(var arr: KBD_BYTE_ARR;
			      var rda: KBD_BOOL_ARR; id: int16): uint8;
var	ret,
	state	: uint8;
	read	: boolean;
begin
	ret	:= 0;
	state	:= arr[id];
        read	:= rda[id];
        if state = KBD_DOWN then
        begin
        	if read then ret := KBD_DOWN else ret := KBD_PRESSED
        end
        else
        if state = KBD_UP then
        begin
        	if read then ret := KBD_UP else ret := KBD_RELEASED;
        end;
        rda[id] := TRUE;
        KBD_get_value := ret;
end;

procedure       KBD_init;
var	i	: int16;
begin
	for i := 0 to KBD_BUFFER_SIZE-1 do
        begin
                KBD_old_nrm[i]	:= 255;
                KBD_old_ext[i]	:= 255;
                KBD_nrm_read[i]	:= FALSE;
                KBD_ext_read[i]	:= FALSE;
        end;
        for i := 0 to KBD_MAX_BUTTONS-1 do KBD_buttons[i] := 0;

        getintvec($09, KBD_old_handler);
        setintvec($09, addr(KBD_handler));
end;

procedure	KBD_destroy;
begin
        setintvec($09, KBD_old_handler);
{$ifndef __TMT__}
        KBD_old_handler := nil;
{$endif}
end;

function	KBD_get_key(id: int16): int16;
begin
	if id > KBD_BUFFER_SIZE then
        begin
        	KBD_get_key := KBD_UP;
                exit;
        end;
        KBD_get_key := KBD_get_value(KBD_nrm_keys, KBD_nrm_read, id);
end;

function	KBD_get_arrow_key(id: int16): int16;
begin
	if id > 4 then
        begin
        	KBD_get_arrow_key := KBD_UP;
                exit;
        end;
        KBD_get_arrow_key := KBD_get_value(KBD_ext_keys, KBD_ext_read, KBD_ARROW_CODES[id]);
end;

procedure	KBD_add_button(index, key: int16);
begin
	if (index < 0) or (index >= KBD_MAX_BUTTONS) then exit;
        KBD_buttons[index] := key;
end;

function	KBD_get_button(id: int16): int16;
begin
	if (id < 0) or (id >= KBD_MAX_BUTTONS) then
        begin
        	KBD_get_button := KBD_UP;
                exit;
        end;
        KBD_get_button := KBD_get_key(KBD_buttons[id]);
end;

{$endif}