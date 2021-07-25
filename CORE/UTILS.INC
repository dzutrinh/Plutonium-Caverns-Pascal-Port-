{*--------------------------------------------------------------------------*}
{*                            U T I L S . I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Utility routines for the core game engine                   *}
{* Prequisites: None                                                        *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Trinh D.D. Nguyen, 2021                                     *}
{*              Jani Nyk„nen is the author of the neg_mod() function        *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_UTILS__}
{$define __CORE_UTILS__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	F_EOF		= -1;
	F_SEEK_SET	= 1;
        F_SEEK_CUR	= 2;
        F_SEEK_END	= 3;

{ MEMORY MANAGEMENT }
function        MEM_malloc(size: uint16): pointer;		forward;
procedure	MEM_free  (var pt);                     	forward;
procedure       MEM_set   (var pt; value, len: uint16);		forward;
procedure	MEM_copy  (var dst, src; len: uint16); 		forward;

{ FILE HANDLING API }
function        file_open(var f: FILE; create: boolean; name: string): boolean;	forward;
procedure	file_close(var f: FILE);                                        forward;
function	file_eof(var f: FILE): boolean;                                 forward;
function	file_read(var buf; size: uint16; var f: FILE): uint16;          forward;
function	file_write(var buf; size: uint16; var f: FILE): uint16;         forward;
function	file_getc(var f: FILE): int16;                                  forward;
function	file_putc(v: uint8; var f: FILE): boolean;                      forward;
function	file_seek(var f: FILE; pos: uint16; mode: uint8): boolean;      forward;
function	file_tell(var f: FILE): uint16;                                 forward;
function	file_remove(filename: string): boolean;                         forward;

{ UTILITIES }
function	maxi(x, y: int16): int16;    		forward;
function	mini(x, y: int16): int16;               forward;
function	absi(a: int16): int16;			forward;
function	neg_mod(m, n: int16): int16;            forward;
function	ltoa(v: int16): string;                 forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

{--- replicate the C based memory allocator/deallocator }

{ allocate a block of memory on heap, returns the pointer to allocate block }
function        MEM_malloc(size: uint16): pointer;
var	p	: pointer;
begin
	MEM_malloc := nil;

        { need 2 extra bytes to store the size of the memory block }
        size := size + sizeof(uint16);

	if memavail < size then exit;	{ memory check             }
	getmem(p, size);		{ allocate the block       }
        move(size, p^, sizeof(uint16));	{ save the original size   }
        p := pchar(p) + sizeof(uint16); { point to the actual data }
        MEM_malloc := p;		{ and return the pointer   }
end;

{ release the memory occuppied by a block to the heap }
procedure	MEM_free(var pt);
var     size	: uint16;
	p	: pchar;
begin
	p := pchar(pt);
	if p = nil then exit;		{ validate the pointer            }
        p := p - sizeof(uint16);	{ point back to the full block    }
        move(p^, size, sizeof(uint16));	{ and retrieve the allocated size }
        freemem(p, size);		{ free the memory block           }
        p := nil;			{ set the pointer to 'nil'        }
end;

{ fill a block of memory of length len to value }
procedure       MEM_set(var pt; value, len: uint16);
begin
	fillchar(pt, len, value);
end;

{ copy len bytes of data from src memory block to dst }
procedure	MEM_copy(var dst, src; len: uint16);
begin
        move(src, dst, len);
end;

{--- easy file R/W }

function        file_open(var f: FILE; create: boolean; name: string): boolean;
begin
	assign(f, name);
        {$I-}
        if create then rewrite(f, 1) else reset(f, 1);
        {$I+}
        file_open := ioresult = 0;
end;

procedure	file_close(var f: FILE);
begin
	close(f);
end;

function	file_eof(var f: FILE): boolean;
begin
	file_eof := eof(f);
end;

function	file_read(var buf; size: uint16; var f: FILE): uint16;
var	nr	: uint16;
begin
	{$I-}
	blockread(f, buf, size, nr);
        {$I+}
        if ioresult <> 0 then file_read := 0 else file_read := nr;
end;

function	file_write(var buf; size: uint16; var f: FILE): uint16;
var	nw	: uint16;
begin
	{$I-}
	blockwrite(f, buf, size, nw);
        {$I+}
        if ioresult <> 0 then file_write := 0 else file_write := nw;
end;

function	file_getc(var f: FILE): int16;
var	v	: uint8;
begin
        if file_read(v, sizeof(uint8), f) = sizeof(uint8) then
		file_getc := v
	else	file_getc := F_EOF;
end;

function	file_putc(v: byte; var f: FILE): boolean;
begin
	file_putc := file_write(v, 1, f) = 1;
end;

function	file_seek(var f: FILE; pos: uint16; mode: uint8): boolean;
begin
	case mode of
	F_SEEK_SET:	seek(f, pos);
	F_SEEK_CUR:	seek(f, filepos(f) + pos);
	F_SEEK_END:	seek(f, filesize(f) - pos);
	end;
end;

function	file_tell(var f: FILE): uint16;
begin
	file_tell := filepos(f);
end;

function	file_remove(filename: string): boolean;
var	f	: FILE;
begin
	assign(f, filename);
        {$I-}
        erase(f);
        {$I+}
        file_remove := ioresult = 0;
end;

{ --- helpers }

function	maxi(x, y: int16): int16;
begin
	if x > y then maxi := x else maxi := y;
end;

function	mini(x, y: int16): int16;
begin
	if x <= y then mini := x else mini := y;
end;

function	ltoa(v: int16): string;
var	s	: string[12];
begin
        str(v, s);
	ltoa := s;
end;

function	absi(a: int16): int16;
begin
	if a >= 0 then absi := a else absi := -a;
end;

{ Author: Jani Nakynen }
function	neg_mod(m, n: int16): int16;
begin
	if m < 0 then	neg_mod := n - (-m mod n)
        else		neg_mod := m mod n;
end;

{$endif}