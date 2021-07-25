uses	crt;

{$i ..\core\types.inc}
{$i ..\core\utils.inc}
{$i ..\game\globals.inc}

procedure       print_text(st: string);
var     i, a,
	f, b	: longint;
	v	: char;
begin
	i := 1; a := $07;
	while i <= length(st) do
	begin
		v := st[i];
		if (v = '~') and
		   (st[i+1] in ['0'..'9', 'A'..'F', 'a'..'f']) and
		   (st[i+2] in ['0'..'9', 'A'..'F', 'a'..'f']) then
		begin
			inc(i); v := upcase(st[i]);
			case v of
			'0'..'9': b := ord(v)-48;
			'A'..'F': b := ord(v)-55;
			else	  b := 0;
			end;

			inc(i); v := upcase(st[i]);
			case v of
			'0'..'9': f := ord(v)-48;
			'A'..'F': f := ord(v)-55;
			else	  f := 0;
			end;
                        a := b shl 4 + f;
		end
		else
		begin
                	textattr := a;
                        write(v);
		end;
		inc(i);
	end;
end;

var	level	: integer;
	f	: FILE;

begin
	print_text('~08+-------------------------+'#13#10);
	print_text('~08| ~0APlutonium Caverns ~02Cheat ~08|'#13#10);
	print_text('~08+-------------------------+'#13#10);
        repeat
                writeln;
        	print_text('~0FLevel to unlock ~07(~0D1~07..~0D17~07), ~0C0 ~04to exit~07: ');
		readln(level);
		if level = 0 then exit;
                if (level < 1) or (level > 17) then
                        print_text('~04ERROR: ~07Invalid input.'#13#10);
        until (level >= 1) and (level <= 17);

        if file_open(f, TRUE, F_SAVE_GAME) then
	begin
                file_write(level, 1, f);
                file_close(f);
                print_text('~03INFO: ~07Save game created.'#13#10);
        end
        else	print_text('~04ERROR: ~07Unable to save game file.'#13#10);
end.
