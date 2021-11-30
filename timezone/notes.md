This is the JMP scripting language JSL that builds the timezone blob that is stored in the ESP32 to lookup a timezone offset from a lat-lon value from the GPS.

I've run it by hand several times; good luck. It will be a pain to rewrite in Python because it depends on JMP graphics.

a recent run (20Oct2021:23:32:36) made this log. 

/*:
The FileSnapper add-in removed 0 snapshots older than 1000 days 
and keeps 4512 snapshots (oldest is now 286 days old.)
The FileSnapper add-in used 4.5 seconds during startup.
That was slow. Consider using less than 1000 days for the snapshot Retention.

//:*/

dtTrans = open("https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.csv",Import Settings(

		End Of Line( CRLF, CR, LF ),

		End Of Field( Comma, CSV( 1 ) ),

		Strip Quotes( 1 ),

		Use Apostrophe as Quotation Mark( 0 ),

		Use Regional Settings( 0 ),

		Scan Whole File( 1 ),

		Treat empty columns as numeric( 0 ),

		CompressNumericColumns( 0 ),

		CompressCharacterColumns( 0 ),

		CompressAllowListCheck( 0 ),

		Labels( 0 ),

		Column Names Start( 1 ),

		Data Starts( 1 ),

		Lines To Read( "All" ),

		Year Rule( "20xx" )

	));
/*:

filesnapper watchdog callback

filesnapper watchdog callback

filesnapper watchdog callback

Data Table( "zones" )
//:*/
	za = Open("https://github.com/evansiroky/timezone-boundary-builder/releases/download/2020d/timezones.geojson.zip", "zip" );
/*:

Open( "C:\!\Users\!\v1\!\AppData\!\Local\!\Temp\!\timezones.geojson1.zip", zip )
//:*/
	blob = za << read( "combined.json", Format( blob ) );
/*:

Blob( 135523620 bytes ) assigned.

//:*/
	p = Parse JSON( Blob To Char( blob ) ); // this grinds for a bit, it is 135MB
/*:

Associative array( 2 elements ) assigned.

//:*/
	Save Text File( "$desktop/rlejson.jsl", char(p) ); // this is what is needed next time
/*:

"C:\Users\v1\Desktop\rlejson.jsl"
//:*/
P = Eval( Parse( Load Text File( "$desktop/rlejson.jsl" ) ) ); // loads faster this way
/*:

Associative array( 2 elements ) assigned.

//:*/
featureList = P["features"]; // list of 426 AAs (no water)
/*:

List( 426 elements ) assigned.

//:*/
freqCodeToName={"water"}; 
/*:

{"water"}
//:*/

iFeatureToName = {};

for( global:iFeature = 1, global:iFeature <= N Items( featureList ), global:iFeature += 1,

	tzdata = featureList[global:iFeature]; // aa {"geometry", "properties", "type"}

	tzproperties = tzdata["properties"]; // aa ["tzid" => "Africa/Abidjan"]

	tzname=tzproperties["tzid"];

	insertinto(iFeatureToName,tzname);

	if(!contains(freqCodeToName,tzname),

		write("\!nadd ",tzname);

		insertinto(freqCodeToName, tzname); // see comment above about "freqCodeToName={"water"};" -- this makes the uncompressed version

	);

);


/*:

add Africa/Abidjan
add Africa/Accra
add Africa/Addis_Ababa
add Africa/Algiers
add Africa/Asmara
add Africa/Bamako
add Africa/Bangui
add Africa/Banjul
add Africa/Bissau
add Africa/Blantyre
add Africa/Brazzaville
add Africa/Bujumbura
add Africa/Cairo
add Africa/Casablanca
add Africa/Ceuta
add Africa/Conakry
add Africa/Dakar
add Africa/Dar_es_Salaam
add Africa/Djibouti
add Africa/Douala
add Africa/El_Aaiun
add Africa/Freetown
add Africa/Gaborone
add Africa/Harare
add Africa/Johannesburg
add Africa/Juba
add Africa/Kampala
add Africa/Khartoum
add Africa/Kigali
add Africa/Kinshasa
add Africa/Lagos
add Africa/Libreville
add Africa/Lome
add Africa/Luanda
add Africa/Lubumbashi
add Africa/Lusaka
add Africa/Malabo
add Africa/Maputo
add Africa/Maseru
add Africa/Mbabane
add Africa/Mogadishu
add Africa/Monrovia
add Africa/Nairobi
add Africa/Ndjamena
add Africa/Niamey
add Africa/Nouakchott
add Africa/Ouagadougou
add Africa/Porto-Novo
add Africa/Sao_Tome
add Africa/Tripoli
add Africa/Tunis
add Africa/Windhoek
add America/Adak
add America/Anchorage
add America/Anguilla
add America/Antigua
add America/Aruba
add America/Araguaina
add America/Argentina/Buenos_Aires
add America/Argentina/Catamarca
add America/Argentina/Cordoba
add America/Argentina/Jujuy
add America/Argentina/La_Rioja
add America/Argentina/Mendoza
add America/Argentina/Rio_Gallegos
add America/Argentina/Salta
add America/Argentina/San_Juan
add America/Argentina/San_Luis
add America/Argentina/Tucuman
add America/Argentina/Ushuaia
add America/Asuncion
add America/Atikokan
add America/Bahia
add America/Bahia_Banderas
add America/Barbados
add America/Belem
add America/Belize
add America/Blanc-Sablon
add America/Boa_Vista
add America/Bogota
add America/Boise
add America/Cambridge_Bay
add America/Campo_Grande
add America/Cancun
add America/Caracas
add America/Cayenne
add America/Cayman
add America/Chicago
add America/Chihuahua
add America/Costa_Rica
add America/Creston
add America/Cuiaba
add America/Curacao
add America/Danmarkshavn
add America/Dawson
add America/Dawson_Creek
add America/Denver
add America/Detroit
add America/Dominica
add America/Edmonton
add America/Eirunepe
add America/El_Salvador
add America/Fort_Nelson
add America/Fortaleza
add America/Glace_Bay
add America/Goose_Bay
add America/Grand_Turk
add America/Grenada
add America/Guadeloupe
add America/Guatemala
add America/Guayaquil
add America/Guyana
add America/Halifax
add America/Havana
add America/Hermosillo
add America/Indiana/Indianapolis
add America/Indiana/Knox
add America/Indiana/Marengo
add America/Indiana/Petersburg
add America/Indiana/Tell_City
add America/Indiana/Vevay
add America/Indiana/Vincennes
add America/Indiana/Winamac
add America/Inuvik
add America/Iqaluit
add America/Jamaica
add America/Juneau
add America/Kentucky/Louisville
add America/Kentucky/Monticello
add America/Kralendijk
add America/La_Paz
add America/Lima
add America/Los_Angeles
add America/Lower_Princes
add America/Maceio
add America/Managua
add America/Manaus
add America/Marigot
add America/Martinique
add America/Matamoros
add America/Mazatlan
add America/Miquelon
add America/Menominee
add America/Merida
add America/Metlakatla
add America/Mexico_City
add America/Moncton
add America/Monterrey
add America/Montevideo
add America/Montserrat
add America/Nassau
add America/New_York
add America/Nipigon
add America/Nome
add America/Noronha
add America/North_Dakota/Beulah
add America/North_Dakota/Center
add America/North_Dakota/New_Salem
add America/Nuuk
add America/Ojinaga
add America/Panama
add America/Pangnirtung
add America/Paramaribo
add America/Phoenix
add America/Port-au-Prince
add America/Port_of_Spain
add America/Porto_Velho
add America/Puerto_Rico
add America/Punta_Arenas
add America/Rainy_River
add America/Rankin_Inlet
add America/Recife
add America/Regina
add America/Resolute
add America/Rio_Branco
add America/Santarem
add America/Santiago
add America/Santo_Domingo
add America/Sao_Paulo
add America/Scoresbysund
add America/Sitka
add America/St_Barthelemy
add America/St_Johns
add America/St_Kitts
add America/St_Lucia
add America/St_Thomas
add America/St_Vincent
add America/Swift_Current
add America/Tegucigalpa
add America/Thule
add America/Thunder_Bay
add America/Tijuana
add America/Toronto
add America/Tortola
add America/Vancouver
add America/Whitehorse
add America/Winnipeg
add America/Yakutat
add America/Yellowknife
add Antarctica/Casey
add Antarctica/Davis
add Antarctica/DumontDUrville
add Antarctica/Macquarie
add Antarctica/Mawson
add Antarctica/McMurdo
add Antarctica/Palmer
add Antarctica/Rothera
add Antarctica/Syowa
add Antarctica/Troll
add Antarctica/Vostok
add Arctic/Longyearbyen
add Asia/Aden
add Asia/Almaty
add Asia/Amman
add Asia/Anadyr
add Asia/Aqtau
add Asia/Aqtobe
add Asia/Ashgabat
add Asia/Atyrau
add Asia/Baghdad
add Asia/Bahrain
add Asia/Baku
add Asia/Bangkok
add Asia/Barnaul
add Asia/Beirut
add Asia/Bishkek
add Asia/Brunei
add Asia/Chita
add Asia/Choibalsan
add Asia/Colombo
add Asia/Damascus
add Asia/Dhaka
add Asia/Dili
add Asia/Dubai
add Asia/Dushanbe
add Asia/Famagusta
add Asia/Gaza
add Asia/Hebron
add Asia/Ho_Chi_Minh
add Asia/Hong_Kong
add Asia/Hovd
add Asia/Irkutsk
add Asia/Jakarta
add Asia/Jayapura
add Asia/Jerusalem
add Asia/Kabul
add Asia/Kamchatka
add Asia/Karachi
add Asia/Kathmandu
add Asia/Khandyga
add Asia/Kolkata
add Asia/Krasnoyarsk
add Asia/Kuala_Lumpur
add Asia/Kuching
add Asia/Kuwait
add Asia/Macau
add Asia/Magadan
add Asia/Makassar
add Asia/Manila
add Asia/Muscat
add Asia/Nicosia
add Asia/Novokuznetsk
add Asia/Novosibirsk
add Asia/Omsk
add Asia/Oral
add Asia/Phnom_Penh
add Asia/Pontianak
add Asia/Pyongyang
add Asia/Qatar
add Asia/Qostanay
add Asia/Qyzylorda
add Asia/Riyadh
add Asia/Samarkand
add Asia/Sakhalin
add Asia/Seoul
add Asia/Shanghai
add Asia/Singapore
add Asia/Srednekolymsk
add Asia/Taipei
add Asia/Tashkent
add Asia/Tbilisi
add Asia/Tehran
add Asia/Thimphu
add Asia/Tokyo
add Asia/Tomsk
add Asia/Ulaanbaatar
add Asia/Urumqi
add Asia/Ust-Nera
add Asia/Vientiane
add Asia/Vladivostok
add Asia/Yakutsk
add Asia/Yangon
add Asia/Yekaterinburg
add Asia/Yerevan
add Atlantic/Azores
add Atlantic/Bermuda
add Atlantic/Canary
add Atlantic/Cape_Verde
add Atlantic/Faroe
add Atlantic/Madeira
add Atlantic/Reykjavik
add Atlantic/South_Georgia
add Atlantic/St_Helena
add Atlantic/Stanley
add Australia/Adelaide
add Australia/Brisbane
add Australia/Broken_Hill
add Australia/Currie
add Australia/Darwin
add Australia/Eucla
add Australia/Hobart
add Australia/Lindeman
add Australia/Lord_Howe
add Australia/Melbourne
add Australia/Perth
add Australia/Sydney
add Etc/UTC
add Europe/Amsterdam
add Europe/Andorra
add Europe/Astrakhan
add Europe/Athens
add Europe/Belgrade
add Europe/Berlin
add Europe/Bratislava
add Europe/Brussels
add Europe/Bucharest
add Europe/Budapest
add Europe/Busingen
add Europe/Chisinau
add Europe/Copenhagen
add Europe/Dublin
add Europe/Gibraltar
add Europe/Guernsey
add Europe/Helsinki
add Europe/Isle_of_Man
add Europe/Istanbul
add Europe/Jersey
add Europe/Kaliningrad
add Europe/Kiev
add Europe/Kirov
add Europe/Lisbon
add Europe/Ljubljana
add Europe/London
add Europe/Luxembourg
add Europe/Madrid
add Europe/Malta
add Europe/Mariehamn
add Europe/Minsk
add Europe/Monaco
add Europe/Moscow
add Europe/Oslo
add Europe/Paris
add Europe/Podgorica
add Europe/Prague
add Europe/Riga
add Europe/Rome
add Europe/Samara
add Europe/San_Marino
add Europe/Sarajevo
add Europe/Saratov
add Europe/Simferopol
add Europe/Skopje
add Europe/Sofia
add Europe/Stockholm
add Europe/Tallinn
add Europe/Tirane
add Europe/Ulyanovsk
add Europe/Uzhgorod
add Europe/Vaduz
add Europe/Vatican
add Europe/Vienna
add Europe/Vilnius
add Europe/Volgograd
add Europe/Warsaw
add Europe/Zagreb
add Europe/Zaporozhye
add Europe/Zurich
add Indian/Antananarivo
add Indian/Chagos
add Indian/Christmas
add Indian/Cocos
add Indian/Comoro
add Indian/Kerguelen
add Indian/Mahe
add Indian/Maldives
add Indian/Mauritius
add Indian/Mayotte
add Indian/Reunion
add Pacific/Apia
add Pacific/Auckland
add Pacific/Bougainville
add Pacific/Chatham
add Pacific/Chuuk
add Pacific/Easter
add Pacific/Efate
add Pacific/Enderbury
add Pacific/Fakaofo
add Pacific/Fiji
add Pacific/Funafuti
add Pacific/Galapagos
add Pacific/Gambier
add Pacific/Guadalcanal
add Pacific/Guam
add Pacific/Honolulu
add Pacific/Kiritimati
add Pacific/Kosrae
add Pacific/Kwajalein
add Pacific/Majuro
add Pacific/Marquesas
add Pacific/Midway
add Pacific/Nauru
add Pacific/Niue
add Pacific/Norfolk
add Pacific/Noumea
add Pacific/Pago_Pago
add Pacific/Palau
add Pacific/Pitcairn
add Pacific/Pohnpei
add Pacific/Port_Moresby
add Pacific/Rarotonga
add Pacific/Saipan
add Pacific/Tahiti
add Pacific/Tarawa
add Pacific/Tongatapu
add Pacific/Wake
add Pacific/Wallis

//:*/

if( freqCodeToName[1] != "water" | freqCodeToName[2] != "Asia/Shanghai" | freqCodeToName[426] != "Europe/Busingen" | freqCodeToName[427] != "Europe/Vatican",

	Throw( "freqCodeToName" ) // probably comment this out on the first run which won't be sorted, and maybe later if the world changes.

);
/*:

freqCodeToName

//:*/
freqCodeToName
/*:

{"water", "Africa/Abidjan", "Africa/Accra", "Africa/Addis_Ababa", "Africa/Algiers",
"Africa/Asmara", "Africa/Bamako", "Africa/Bangui", "Africa/Banjul", "Africa/Bissau",
"Africa/Blantyre", "Africa/Brazzaville", "Africa/Bujumbura", "Africa/Cairo",
"Africa/Casablanca", "Africa/Ceuta", "Africa/Conakry", "Africa/Dakar",
"Africa/Dar_es_Salaam", "Africa/Djibouti", "Africa/Douala", "Africa/El_Aaiun",
"Africa/Freetown", "Africa/Gaborone", "Africa/Harare", "Africa/Johannesburg",
"Africa/Juba", "Africa/Kampala", "Africa/Khartoum", "Africa/Kigali",
"Africa/Kinshasa", "Africa/Lagos", "Africa/Libreville", "Africa/Lome",
"Africa/Luanda", "Africa/Lubumbashi", "Africa/Lusaka", "Africa/Malabo",
"Africa/Maputo", "Africa/Maseru", "Africa/Mbabane", "Africa/Mogadishu",
"Africa/Monrovia", "Africa/Nairobi", "Africa/Ndjamena", "Africa/Niamey",
"Africa/Nouakchott", "Africa/Ouagadougou", "Africa/Porto-Novo", "Africa/Sao_Tome",
"Africa/Tripoli", "Africa/Tunis", "Africa/Windhoek", "America/Adak",
"America/Anchorage", "America/Anguilla", "America/Antigua", "America/Aruba",
"America/Araguaina", "America/Argentina/Buenos_Aires", "America/Argentina/Catamarca",
"America/Argentina/Cordoba", "America/Argentina/Jujuy", "America/Argentina/La_Rioja",
"America/Argentina/Mendoza", "America/Argentina/Rio_Gallegos",
"America/Argentina/Salta", "America/Argentina/San_Juan",
"America/Argentina/San_Luis", "America/Argentina/Tucuman",
"America/Argentina/Ushuaia", "America/Asuncion", "America/Atikokan", "America/Bahia",
"America/Bahia_Banderas", "America/Barbados", "America/Belem", "America/Belize",
"America/Blanc-Sablon", "America/Boa_Vista", "America/Bogota", "America/Boise",
"America/Cambridge_Bay", "America/Campo_Grande", "America/Cancun", "America/Caracas",
"America/Cayenne", "America/Cayman", "America/Chicago", "America/Chihuahua",
"America/Costa_Rica", "America/Creston", "America/Cuiaba", "America/Curacao",
"America/Danmarkshavn", "America/Dawson", "America/Dawson_Creek", "America/Denver",
"America/Detroit", "America/Dominica", "America/Edmonton", "America/Eirunepe",
"America/El_Salvador", "America/Fort_Nelson", "America/Fortaleza",
"America/Glace_Bay", "America/Goose_Bay", "America/Grand_Turk", "America/Grenada",
"America/Guadeloupe", "America/Guatemala", "America/Guayaquil", "America/Guyana",
"America/Halifax", "America/Havana", "America/Hermosillo",
"America/Indiana/Indianapolis", "America/Indiana/Knox", "America/Indiana/Marengo",
"America/Indiana/Petersburg", "America/Indiana/Tell_City", "America/Indiana/Vevay",
"America/Indiana/Vincennes", "America/Indiana/Winamac", "America/Inuvik",
"America/Iqaluit", "America/Jamaica", "America/Juneau",
"America/Kentucky/Louisville", "America/Kentucky/Monticello", "America/Kralendijk",
"America/La_Paz", "America/Lima", "America/Los_Angeles", "America/Lower_Princes",
"America/Maceio", "America/Managua", "America/Manaus", "America/Marigot",
"America/Martinique", "America/Matamoros", "America/Mazatlan", "America/Miquelon",
"America/Menominee", "America/Merida", "America/Metlakatla", "America/Mexico_City",
"America/Moncton", "America/Monterrey", "America/Montevideo", "America/Montserrat",
"America/Nassau", "America/New_York", "America/Nipigon", "America/Nome",
"America/Noronha", "America/North_Dakota/Beulah", "America/North_Dakota/Center",
"America/North_Dakota/New_Salem", "America/Nuuk", "America/Ojinaga",
"America/Panama", "America/Pangnirtung", "America/Paramaribo", "America/Phoenix",
"America/Port-au-Prince", "America/Port_of_Spain", "America/Porto_Velho",
"America/Puerto_Rico", "America/Punta_Arenas", "America/Rainy_River",
"America/Rankin_Inlet", "America/Recife", "America/Regina", "America/Resolute",
"America/Rio_Branco", "America/Santarem", "America/Santiago",
"America/Santo_Domingo", "America/Sao_Paulo", "America/Scoresbysund",
"America/Sitka", "America/St_Barthelemy", "America/St_Johns", "America/St_Kitts",
"America/St_Lucia", "America/St_Thomas", "America/St_Vincent",
"America/Swift_Current", "America/Tegucigalpa", "America/Thule",
"America/Thunder_Bay", "America/Tijuana", "America/Toronto", "America/Tortola",
"America/Vancouver", "America/Whitehorse", "America/Winnipeg", "America/Yakutat",
"America/Yellowknife", "Antarctica/Casey", "Antarctica/Davis",
"Antarctica/DumontDUrville", "Antarctica/Macquarie", "Antarctica/Mawson",
"Antarctica/McMurdo", "Antarctica/Palmer", "Antarctica/Rothera", "Antarctica/Syowa",
"Antarctica/Troll", "Antarctica/Vostok", "Arctic/Longyearbyen", "Asia/Aden",
"Asia/Almaty", "Asia/Amman", "Asia/Anadyr", "Asia/Aqtau", "Asia/Aqtobe",
"Asia/Ashgabat", "Asia/Atyrau", "Asia/Baghdad", "Asia/Bahrain", "Asia/Baku",
"Asia/Bangkok", "Asia/Barnaul", "Asia/Beirut", "Asia/Bishkek", "Asia/Brunei",
"Asia/Chita", "Asia/Choibalsan", "Asia/Colombo", "Asia/Damascus", "Asia/Dhaka",
"Asia/Dili", "Asia/Dubai", "Asia/Dushanbe", "Asia/Famagusta", "Asia/Gaza",
"Asia/Hebron", "Asia/Ho_Chi_Minh", "Asia/Hong_Kong", "Asia/Hovd", "Asia/Irkutsk",
"Asia/Jakarta", "Asia/Jayapura", "Asia/Jerusalem", "Asia/Kabul", "Asia/Kamchatka",
"Asia/Karachi", "Asia/Kathmandu", "Asia/Khandyga", "Asia/Kolkata",
"Asia/Krasnoyarsk", "Asia/Kuala_Lumpur", "Asia/Kuching", "Asia/Kuwait", "Asia/Macau",
"Asia/Magadan", "Asia/Makassar", "Asia/Manila", "Asia/Muscat", "Asia/Nicosia",
"Asia/Novokuznetsk", "Asia/Novosibirsk", "Asia/Omsk", "Asia/Oral", "Asia/Phnom_Penh",
"Asia/Pontianak", "Asia/Pyongyang", "Asia/Qatar", "Asia/Qostanay", "Asia/Qyzylorda",
"Asia/Riyadh", "Asia/Samarkand", "Asia/Sakhalin", "Asia/Seoul", "Asia/Shanghai",
"Asia/Singapore", "Asia/Srednekolymsk", "Asia/Taipei", "Asia/Tashkent",
"Asia/Tbilisi", "Asia/Tehran", "Asia/Thimphu", "Asia/Tokyo", "Asia/Tomsk",
"Asia/Ulaanbaatar", "Asia/Urumqi", "Asia/Ust-Nera", "Asia/Vientiane",
"Asia/Vladivostok", "Asia/Yakutsk", "Asia/Yangon", "Asia/Yekaterinburg",
"Asia/Yerevan", "Atlantic/Azores", "Atlantic/Bermuda", "Atlantic/Canary",
"Atlantic/Cape_Verde", "Atlantic/Faroe", "Atlantic/Madeira", "Atlantic/Reykjavik",
"Atlantic/South_Georgia", "Atlantic/St_Helena", "Atlantic/Stanley",
"Australia/Adelaide", "Australia/Brisbane", "Australia/Broken_Hill",
"Australia/Currie", "Australia/Darwin", "Australia/Eucla", "Australia/Hobart",
"Australia/Lindeman", "Australia/Lord_Howe", "Australia/Melbourne",
"Australia/Perth", "Australia/Sydney", "Etc/UTC", "Europe/Amsterdam",
"Europe/Andorra", "Europe/Astrakhan", "Europe/Athens", "Europe/Belgrade",
"Europe/Berlin", "Europe/Bratislava", "Europe/Brussels", "Europe/Bucharest",
"Europe/Budapest", "Europe/Busingen", "Europe/Chisinau", "Europe/Copenhagen",
"Europe/Dublin", "Europe/Gibraltar", "Europe/Guernsey", "Europe/Helsinki",
"Europe/Isle_of_Man", "Europe/Istanbul", "Europe/Jersey", "Europe/Kaliningrad",
"Europe/Kiev", "Europe/Kirov", "Europe/Lisbon", "Europe/Ljubljana", "Europe/London",
"Europe/Luxembourg", "Europe/Madrid", "Europe/Malta", "Europe/Mariehamn",
"Europe/Minsk", "Europe/Monaco", "Europe/Moscow", "Europe/Oslo", "Europe/Paris",
"Europe/Podgorica", "Europe/Prague", "Europe/Riga", "Europe/Rome", "Europe/Samara",
"Europe/San_Marino", "Europe/Sarajevo", "Europe/Saratov", "Europe/Simferopol",
"Europe/Skopje", "Europe/Sofia", "Europe/Stockholm", "Europe/Tallinn",
"Europe/Tirane", "Europe/Ulyanovsk", "Europe/Uzhgorod", "Europe/Vaduz",
"Europe/Vatican", "Europe/Vienna", "Europe/Vilnius", "Europe/Volgograd",
"Europe/Warsaw", "Europe/Zagreb", "Europe/Zaporozhye", "Europe/Zurich",
"Indian/Antananarivo", "Indian/Chagos", "Indian/Christmas", "Indian/Cocos",
"Indian/Comoro", "Indian/Kerguelen", "Indian/Mahe", "Indian/Maldives",
"Indian/Mauritius", "Indian/Mayotte", "Indian/Reunion", "Pacific/Apia",
"Pacific/Auckland", "Pacific/Bougainville", "Pacific/Chatham", "Pacific/Chuuk",
"Pacific/Easter", "Pacific/Efate", "Pacific/Enderbury", "Pacific/Fakaofo",
"Pacific/Fiji", "Pacific/Funafuti", "Pacific/Galapagos", "Pacific/Gambier",
"Pacific/Guadalcanal", "Pacific/Guam", "Pacific/Honolulu", "Pacific/Kiritimati",
"Pacific/Kosrae", "Pacific/Kwajalein", "Pacific/Majuro", "Pacific/Marquesas",
"Pacific/Midway", "Pacific/Nauru", "Pacific/Niue", "Pacific/Norfolk",
"Pacific/Noumea", "Pacific/Pago_Pago", "Pacific/Palau", "Pacific/Pitcairn",
"Pacific/Pohnpei", "Pacific/Port_Moresby", "Pacific/Rarotonga", "Pacific/Saipan",
"Pacific/Tahiti", "Pacific/Tarawa", "Pacific/Tongatapu", "Pacific/Wake",
"Pacific/Wallis"}
//:*/

dopoly = function( {tzName, tzcoords},

	{i, m, xs,ys},

	for( i = 1, i <= N Items( tzcoords ), i += 1, 

		/*

		I'm still not sure what the 2,3,... polys represent. Holes maybe, 

		in a single disjoint poly. Render same color, smaller polys render later (2nd run).

		

		this is part of the data that is not right with the json wizard (comments at top)

		*/

		//if(i>1,fillcolor("magenta")); 

		m = Matrix( tzcoords[i] );

		xs = m[0, 1];

		ys = m[0, 2];

		//pencolor("white");pensize(1);

		polygon/*line*/(xs, ys);// xmatrix,ymatrix

	)

);



/*

drawpolys does all the disjoint parts of one timezone; a timezone with only 1 part is not nested

quite as deep as a timezone with multiple disjoint parts.

*/

drawpolys = function( {}, // 

	{tzdata, tzgeometry, tzproperties, tztype, tzGeometryType, tzcoordinates, code, greenbits, bluebits, redbits, ipoly},

	tzdata = featureList[global:iFeature]; // aa {"geometry", "properties", "type"}

	tzgeometry = tzdata["geometry"]; // aa {"coordinates", "type"}

	tzproperties = tzdata["properties"]; // aa ["tzid" => "Africa/Abidjan"]

	/* sanity checking, these should match the log line that follows from the driver loop below */

	write("\!n drawing ",global:iFeature,": ", tzproperties["tzid"] );

	tztype = tzdata["type"]; // string "Feature"

	tzGeometryType = tzgeometry["type"];//"Polygon" or "MultiPolygon"

	tzcoordinates = tzgeometry["coordinates"]; // list

	Fill Color( "white" );

	if(

		tzGeometryType == "MultiPolygon", /* a disjoint time zone */

			for( ipoly = 1, ipoly <= N Items( tzcoordinates ), ipoly += 1,

				dopoly( tzproperties["tzid"], tzcoordinates[ipoly] ) /* deeper nest */

			);//

	, /*else if*/tzGeometryType == "Polygon", /* single poly timezone */

			dopoly( tzproperties["tzid"], tzcoordinates );// no extra nest

	, /*else*/

		Throw( "tzGeometryType" || Char( tzGeometryType ) )

	);

);

/*:

Function( {},
    {tzdata, tzgeometry, tzproperties, tztype, tzGeometryType, tzcoordinates, code,
    greenbits, bluebits, redbits, ipoly},
    tzdata = featureList[global:iFeature];
    tzgeometry = tzdata["geometry"];
    tzproperties = tzdata["properties"];
    Write( "
 drawing ", global:iFeature, ": ", tzproperties["tzid"] );
    tztype = tzdata["type"];
    tzGeometryType = tzgeometry["type"];
    tzcoordinates = tzgeometry["coordinates"];
    Fill Color( "white" );
    If(
        tzGeometryType == "MultiPolygon",
            For( ipoly = 1, ipoly <= N Items( tzcoordinates ), ipoly += 1,
                dopoly( tzproperties["tzid"], tzcoordinates[ipoly] )
            ),
        tzGeometryType == "Polygon", dopoly( tzproperties["tzid"], tzcoordinates ),
        Throw( "tzGeometryType" || Char( tzGeometryType ) )
    );
)
//:*/

composition = 0;

composition = J( 16000, 30000, 0 );42;

/*:

42
//:*/
	for( ifreq = 2/* 1 is water */, ifreq <= N Items( freqCodeToName ), ifreq += 1,

		/*

		this is the big-to-small list of timezones; get the name of the zone to do next...

		*/

		thisname=freqCodeToName[ifreq];

		/*

		this global variable tells the poly routine what to work on; locate the name and use the index from loc

		*/

		global:iFeature = loc(iFeatureToName,thisname)[1];// draw smallest last

		/*

		Surprisingly nothing happens when the graphbox is made without a window, until...

		*/

		gb = Graph Box(

			X Scale( -180, 180 ),

			Y Scale( -90, 90 ),

			framesize( 30000 + 1, 16000 + 1 ), // drawing beyond 30,000 may not always work, 16x30 is about as big as it can be.

			<<backgroundcolor( "black" ),

			drawpolys();

			//marker(colorstate("green"),{-180,-90},{180,-90},{-180,90},{180,90},{0,0}); // corner markers might help figure out trimming, below

		);

		/*

		...the picture/bitmap is requested. That takes some time.

		*/

		pixels = gb[framebox( 1 )] << getpicture; // 30 seconds to make the picture

		gb = 0;

		pixels = pixels << getpixels; // 10 seconds to convert to pixel matrix

		/*

		trimming the image. added 1 in the graphbox to get the data inside a 30000x16000 

		(not 29999x15999) space, now strip off the extra frame pixels

		*/

		pixels = pixels[2 :: (N Rows( pixels ) - 5), 2 :: (N Cols( pixels ) - 5)];

		/*

		after another small sanity check, find the nonzero (rendered white timezone on black background)

		pixels and add them to the composition as *** global:iFeature *** value.

		iFeature will become the RLE code in the next step.

		*/

		if( N Rows( composition ) == N Rows( pixels ) & N Cols( composition ) == N Cols( pixels ),

			composition[Loc( pixels )] = global:iFeature; // 

		, // else

			Throw( "composition" )

		);

		write("\!n freq=", ifreq," composition code=",global:iFeature," for ",thisname );

		if(thisname == "America/New_York", write(" ***"));

		Wait( .1 );

	);

	/*

	delete the old file saving the composition, then recreate it.

	My computer was unable to do it in one save, so I split it 

	into four appends. Probably no real point in the delete since

	the first save is a replace.

	And this hardcodes the 16000 height, but that isn't the dimension

	I wish was bigger; the horizontal resolution is more important for

	timezones, most of the time.

	*/

	try(deletefile("$desktop/rlecomp.jsl"));



	bcomp = matrixtoblob(composition[1::4000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("replace") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[4001::8000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[8001::12000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[12001::16000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));
/*:

 drawing 1: Africa/Abidjan
 freq=2 composition code=1 for Africa/Abidjan
 drawing 2: Africa/Accra
 freq=3 composition code=2 for Africa/Accra
 drawing 3: Africa/Addis_Ababa
 freq=4 composition code=3 for Africa/Addis_Ababa
 drawing 4: Africa/Algiers
 freq=5 composition code=4 for Africa/Algiers
 drawing 5: Africa/Asmara
 freq=6 composition code=5 for Africa/Asmara
 drawing 6: Africa/Bamako
 freq=7 composition code=6 for Africa/Bamako
 drawing 7: Africa/Bangui
 freq=8 composition code=7 for Africa/Bangui
 drawing 8: Africa/Banjul
 freq=9 composition code=8 for Africa/Banjul
 drawing 9: Africa/Bissau
 freq=10 composition code=9 for Africa/Bissau
 drawing 10: Africa/Blantyre
 freq=11 composition code=10 for Africa/Blantyre
 drawing 11: Africa/Brazzaville
 freq=12 composition code=11 for Africa/Brazzaville
 drawing 12: Africa/Bujumbura
 freq=13 composition code=12 for Africa/Bujumbura
 drawing 13: Africa/Cairo
 freq=14 composition code=13 for Africa/Cairo
 drawing 14: Africa/Casablanca
 freq=15 composition code=14 for Africa/Casablanca
 drawing 15: Africa/Ceuta
 freq=16 composition code=15 for Africa/Ceuta
 drawing 16: Africa/Conakry
 freq=17 composition code=16 for Africa/Conakry
 drawing 17: Africa/Dakar
 freq=18 composition code=17 for Africa/Dakar
 drawing 18: Africa/Dar_es_Salaam
 freq=19 composition code=18 for Africa/Dar_es_Salaam
 drawing 19: Africa/Djibouti
 freq=20 composition code=19 for Africa/Djibouti
 drawing 20: Africa/Douala
 freq=21 composition code=20 for Africa/Douala
 drawing 21: Africa/El_Aaiun
 freq=22 composition code=21 for Africa/El_Aaiun
 drawing 22: Africa/Freetown
 freq=23 composition code=22 for Africa/Freetown
 drawing 23: Africa/Gaborone
 freq=24 composition code=23 for Africa/Gaborone
 drawing 24: Africa/Harare
 freq=25 composition code=24 for Africa/Harare
 drawing 25: Africa/Johannesburg
 freq=26 composition code=25 for Africa/Johannesburg
 drawing 26: Africa/Juba
 freq=27 composition code=26 for Africa/Juba
 drawing 27: Africa/Kampala
 freq=28 composition code=27 for Africa/Kampala
 drawing 28: Africa/Khartoum
 freq=29 composition code=28 for Africa/Khartoum
 drawing 29: Africa/Kigali
 freq=30 composition code=29 for Africa/Kigali
 drawing 30: Africa/Kinshasa
 freq=31 composition code=30 for Africa/Kinshasa
 drawing 31: Africa/Lagos
 freq=32 composition code=31 for Africa/Lagos
 drawing 32: Africa/Libreville
 freq=33 composition code=32 for Africa/Libreville
 drawing 33: Africa/Lome
 freq=34 composition code=33 for Africa/Lome
 drawing 34: Africa/Luanda
 freq=35 composition code=34 for Africa/Luanda
 drawing 35: Africa/Lubumbashi
 freq=36 composition code=35 for Africa/Lubumbashi
 drawing 36: Africa/Lusaka
 freq=37 composition code=36 for Africa/Lusaka
 drawing 37: Africa/Malabo
 freq=38 composition code=37 for Africa/Malabo
 drawing 38: Africa/Maputo
 freq=39 composition code=38 for Africa/Maputo
 drawing 39: Africa/Maseru
 freq=40 composition code=39 for Africa/Maseru
 drawing 40: Africa/Mbabane
 freq=41 composition code=40 for Africa/Mbabane
 drawing 41: Africa/Mogadishu
 freq=42 composition code=41 for Africa/Mogadishu
 drawing 42: Africa/Monrovia
 freq=43 composition code=42 for Africa/Monrovia
 drawing 43: Africa/Nairobi
 freq=44 composition code=43 for Africa/Nairobi
 drawing 44: Africa/Ndjamena
 freq=45 composition code=44 for Africa/Ndjamena
 drawing 45: Africa/Niamey
 freq=46 composition code=45 for Africa/Niamey
 drawing 46: Africa/Nouakchott
 freq=47 composition code=46 for Africa/Nouakchott
 drawing 47: Africa/Ouagadougou
 freq=48 composition code=47 for Africa/Ouagadougou
 drawing 48: Africa/Porto-Novo
 freq=49 composition code=48 for Africa/Porto-Novo
 drawing 49: Africa/Sao_Tome
 freq=50 composition code=49 for Africa/Sao_Tome
 drawing 50: Africa/Tripoli
 freq=51 composition code=50 for Africa/Tripoli
 drawing 51: Africa/Tunis
 freq=52 composition code=51 for Africa/Tunis
 drawing 52: Africa/Windhoek
 freq=53 composition code=52 for Africa/Windhoek
 drawing 53: America/Adak
 freq=54 composition code=53 for America/Adak
 drawing 54: America/Anchorage
 freq=55 composition code=54 for America/Anchorage
 drawing 55: America/Anguilla
 freq=56 composition code=55 for America/Anguilla
 drawing 56: America/Antigua
 freq=57 composition code=56 for America/Antigua
 drawing 57: America/Aruba
 freq=58 composition code=57 for America/Aruba
 drawing 58: America/Araguaina
 freq=59 composition code=58 for America/Araguaina
 drawing 59: America/Argentina/Buenos_Aires
 freq=60 composition code=59 for America/Argentina/Buenos_Aires
 drawing 60: America/Argentina/Catamarca
 freq=61 composition code=60 for America/Argentina/Catamarca
 drawing 61: America/Argentina/Cordoba
 freq=62 composition code=61 for America/Argentina/Cordoba
 drawing 62: America/Argentina/Jujuy
 freq=63 composition code=62 for America/Argentina/Jujuy
 drawing 63: America/Argentina/La_Rioja
 freq=64 composition code=63 for America/Argentina/La_Rioja
 drawing 64: America/Argentina/Mendoza
 freq=65 composition code=64 for America/Argentina/Mendoza
 drawing 65: America/Argentina/Rio_Gallegos
 freq=66 composition code=65 for America/Argentina/Rio_Gallegos
 drawing 66: America/Argentina/Salta
 freq=67 composition code=66 for America/Argentina/Salta
 drawing 67: America/Argentina/San_Juan
 freq=68 composition code=67 for America/Argentina/San_Juan
 drawing 68: America/Argentina/San_Luis
 freq=69 composition code=68 for America/Argentina/San_Luis
 drawing 69: America/Argentina/Tucuman
 freq=70 composition code=69 for America/Argentina/Tucuman
 drawing 70: America/Argentina/Ushuaia
 freq=71 composition code=70 for America/Argentina/Ushuaia
 drawing 71: America/Asuncion
 freq=72 composition code=71 for America/Asuncion
 drawing 72: America/Atikokan
 freq=73 composition code=72 for America/Atikokan
 drawing 73: America/Bahia
 freq=74 composition code=73 for America/Bahia
 drawing 74: America/Bahia_Banderas
 freq=75 composition code=74 for America/Bahia_Banderas
 drawing 75: America/Barbados
 freq=76 composition code=75 for America/Barbados
 drawing 76: America/Belem
 freq=77 composition code=76 for America/Belem
 drawing 77: America/Belize
 freq=78 composition code=77 for America/Belize
 drawing 78: America/Blanc-Sablon
 freq=79 composition code=78 for America/Blanc-Sablon
 drawing 79: America/Boa_Vista
 freq=80 composition code=79 for America/Boa_Vista
 drawing 80: America/Bogota
 freq=81 composition code=80 for America/Bogota
 drawing 81: America/Boise
 freq=82 composition code=81 for America/Boise
 drawing 82: America/Cambridge_Bay
 freq=83 composition code=82 for America/Cambridge_Bay
 drawing 83: America/Campo_Grande
 freq=84 composition code=83 for America/Campo_Grande
 drawing 84: America/Cancun
 freq=85 composition code=84 for America/Cancun
 drawing 85: America/Caracas
 freq=86 composition code=85 for America/Caracas
 drawing 86: America/Cayenne
 freq=87 composition code=86 for America/Cayenne
 drawing 87: America/Cayman
 freq=88 composition code=87 for America/Cayman
 drawing 88: America/Chicago
 freq=89 composition code=88 for America/Chicago
 drawing 89: America/Chihuahua
 freq=90 composition code=89 for America/Chihuahua
 drawing 90: America/Costa_Rica
 freq=91 composition code=90 for America/Costa_Rica
 drawing 91: America/Creston
 freq=92 composition code=91 for America/Creston
 drawing 92: America/Cuiaba
 freq=93 composition code=92 for America/Cuiaba
 drawing 93: America/Curacao
 freq=94 composition code=93 for America/Curacao
 drawing 94: America/Danmarkshavn
 freq=95 composition code=94 for America/Danmarkshavn
 drawing 95: America/Dawson
 freq=96 composition code=95 for America/Dawson
 drawing 96: America/Dawson_Creek
 freq=97 composition code=96 for America/Dawson_Creek
 drawing 97: America/Denver
 freq=98 composition code=97 for America/Denver
 drawing 98: America/Detroit
 freq=99 composition code=98 for America/Detroit
 drawing 99: America/Dominica
 freq=100 composition code=99 for America/Dominica
 drawing 100: America/Edmonton
 freq=101 composition code=100 for America/Edmonton
 drawing 101: America/Eirunepe
 freq=102 composition code=101 for America/Eirunepe
 drawing 102: America/El_Salvador
 freq=103 composition code=102 for America/El_Salvador
 drawing 103: America/Fort_Nelson
 freq=104 composition code=103 for America/Fort_Nelson
 drawing 104: America/Fortaleza
 freq=105 composition code=104 for America/Fortaleza
 drawing 105: America/Glace_Bay
 freq=106 composition code=105 for America/Glace_Bay
 drawing 106: America/Goose_Bay
 freq=107 composition code=106 for America/Goose_Bay
 drawing 107: America/Grand_Turk
 freq=108 composition code=107 for America/Grand_Turk
 drawing 108: America/Grenada
 freq=109 composition code=108 for America/Grenada
 drawing 109: America/Guadeloupe
 freq=110 composition code=109 for America/Guadeloupe
 drawing 110: America/Guatemala
 freq=111 composition code=110 for America/Guatemala
 drawing 111: America/Guayaquil
 freq=112 composition code=111 for America/Guayaquil
 drawing 112: America/Guyana
 freq=113 composition code=112 for America/Guyana
 drawing 113: America/Halifax
 freq=114 composition code=113 for America/Halifax
 drawing 114: America/Havana
 freq=115 composition code=114 for America/Havana
 drawing 115: America/Hermosillo
 freq=116 composition code=115 for America/Hermosillo
 drawing 116: America/Indiana/Indianapolis
 freq=117 composition code=116 for America/Indiana/Indianapolis
 drawing 117: America/Indiana/Knox
 freq=118 composition code=117 for America/Indiana/Knox
 drawing 118: America/Indiana/Marengo
 freq=119 composition code=118 for America/Indiana/Marengo
 drawing 119: America/Indiana/Petersburg
 freq=120 composition code=119 for America/Indiana/Petersburg
 drawing 120: America/Indiana/Tell_City
 freq=121 composition code=120 for America/Indiana/Tell_City
 drawing 121: America/Indiana/Vevay
 freq=122 composition code=121 for America/Indiana/Vevay
 drawing 122: America/Indiana/Vincennes
 freq=123 composition code=122 for America/Indiana/Vincennes
 drawing 123: America/Indiana/Winamac
 freq=124 composition code=123 for America/Indiana/Winamac
 drawing 124: America/Inuvik
 freq=125 composition code=124 for America/Inuvik
 drawing 125: America/Iqaluit
 freq=126 composition code=125 for America/Iqaluit
 drawing 126: America/Jamaica
 freq=127 composition code=126 for America/Jamaica
 drawing 127: America/Juneau
 freq=128 composition code=127 for America/Juneau
 drawing 128: America/Kentucky/Louisville
 freq=129 composition code=128 for America/Kentucky/Louisville
 drawing 129: America/Kentucky/Monticello
 freq=130 composition code=129 for America/Kentucky/Monticello
 drawing 130: America/Kralendijk
 freq=131 composition code=130 for America/Kralendijk
 drawing 131: America/La_Paz
 freq=132 composition code=131 for America/La_Paz
 drawing 132: America/Lima
 freq=133 composition code=132 for America/Lima
 drawing 133: America/Los_Angeles
 freq=134 composition code=133 for America/Los_Angeles
 drawing 134: America/Lower_Princes
 freq=135 composition code=134 for America/Lower_Princes
 drawing 135: America/Maceio
 freq=136 composition code=135 for America/Maceio
 drawing 136: America/Managua
 freq=137 composition code=136 for America/Managua
 drawing 137: America/Manaus
 freq=138 composition code=137 for America/Manaus
 drawing 138: America/Marigot
 freq=139 composition code=138 for America/Marigot
 drawing 139: America/Martinique
 freq=140 composition code=139 for America/Martinique
 drawing 140: America/Matamoros
 freq=141 composition code=140 for America/Matamoros
 drawing 141: America/Mazatlan
 freq=142 composition code=141 for America/Mazatlan
 drawing 142: America/Miquelon
 freq=143 composition code=142 for America/Miquelon
 drawing 143: America/Menominee
 freq=144 composition code=143 for America/Menominee
 drawing 144: America/Merida
 freq=145 composition code=144 for America/Merida
 drawing 145: America/Metlakatla
 freq=146 composition code=145 for America/Metlakatla
 drawing 146: America/Mexico_City
 freq=147 composition code=146 for America/Mexico_City
 drawing 147: America/Moncton
 freq=148 composition code=147 for America/Moncton
 drawing 148: America/Monterrey
 freq=149 composition code=148 for America/Monterrey
 drawing 149: America/Montevideo
 freq=150 composition code=149 for America/Montevideo
 drawing 150: America/Montserrat
 freq=151 composition code=150 for America/Montserrat
 drawing 151: America/Nassau
 freq=152 composition code=151 for America/Nassau
 drawing 152: America/New_York
 freq=153 composition code=152 for America/New_York ***
 drawing 153: America/Nipigon
 freq=154 composition code=153 for America/Nipigon
 drawing 154: America/Nome
 freq=155 composition code=154 for America/Nome
 drawing 155: America/Noronha
 freq=156 composition code=155 for America/Noronha
 drawing 156: America/North_Dakota/Beulah
 freq=157 composition code=156 for America/North_Dakota/Beulah
 drawing 157: America/North_Dakota/Center
 freq=158 composition code=157 for America/North_Dakota/Center
 drawing 158: America/North_Dakota/New_Salem
 freq=159 composition code=158 for America/North_Dakota/New_Salem
 drawing 159: America/Nuuk
 freq=160 composition code=159 for America/Nuuk
 drawing 160: America/Ojinaga
 freq=161 composition code=160 for America/Ojinaga
 drawing 161: America/Panama
 freq=162 composition code=161 for America/Panama
 drawing 162: America/Pangnirtung
 freq=163 composition code=162 for America/Pangnirtung
 drawing 163: America/Paramaribo
 freq=164 composition code=163 for America/Paramaribo
 drawing 164: America/Phoenix
 freq=165 composition code=164 for America/Phoenix
 drawing 165: America/Port-au-Prince
 freq=166 composition code=165 for America/Port-au-Prince
 drawing 166: America/Port_of_Spain
 freq=167 composition code=166 for America/Port_of_Spain
 drawing 167: America/Porto_Velho
 freq=168 composition code=167 for America/Porto_Velho
 drawing 168: America/Puerto_Rico
 freq=169 composition code=168 for America/Puerto_Rico
 drawing 169: America/Punta_Arenas
 freq=170 composition code=169 for America/Punta_Arenas
 drawing 170: America/Rainy_River
 freq=171 composition code=170 for America/Rainy_River
 drawing 171: America/Rankin_Inlet
 freq=172 composition code=171 for America/Rankin_Inlet
 drawing 172: America/Recife
 freq=173 composition code=172 for America/Recife
 drawing 173: America/Regina
 freq=174 composition code=173 for America/Regina
 drawing 174: America/Resolute
 freq=175 composition code=174 for America/Resolute
 drawing 175: America/Rio_Branco
 freq=176 composition code=175 for America/Rio_Branco
 drawing 176: America/Santarem
 freq=177 composition code=176 for America/Santarem
 drawing 177: America/Santiago
 freq=178 composition code=177 for America/Santiago
 drawing 178: America/Santo_Domingo
 freq=179 composition code=178 for America/Santo_Domingo
 drawing 179: America/Sao_Paulo
 freq=180 composition code=179 for America/Sao_Paulo
 drawing 180: America/Scoresbysund
 freq=181 composition code=180 for America/Scoresbysund
 drawing 181: America/Sitka
 freq=182 composition code=181 for America/Sitka
 drawing 182: America/St_Barthelemy
 freq=183 composition code=182 for America/St_Barthelemy
 drawing 183: America/St_Johns
 freq=184 composition code=183 for America/St_Johns
 drawing 184: America/St_Kitts
 freq=185 composition code=184 for America/St_Kitts
 drawing 185: America/St_Lucia
 freq=186 composition code=185 for America/St_Lucia
 drawing 186: America/St_Thomas
 freq=187 composition code=186 for America/St_Thomas
 drawing 187: America/St_Vincent
 freq=188 composition code=187 for America/St_Vincent
 drawing 188: America/Swift_Current
 freq=189 composition code=188 for America/Swift_Current
 drawing 189: America/Tegucigalpa
 freq=190 composition code=189 for America/Tegucigalpa
 drawing 190: America/Thule
 freq=191 composition code=190 for America/Thule
 drawing 191: America/Thunder_Bay
 freq=192 composition code=191 for America/Thunder_Bay
 drawing 192: America/Tijuana
 freq=193 composition code=192 for America/Tijuana
 drawing 193: America/Toronto
 freq=194 composition code=193 for America/Toronto
 drawing 194: America/Tortola
 freq=195 composition code=194 for America/Tortola
 drawing 195: America/Vancouver
 freq=196 composition code=195 for America/Vancouver
 drawing 196: America/Whitehorse
 freq=197 composition code=196 for America/Whitehorse
 drawing 197: America/Winnipeg
 freq=198 composition code=197 for America/Winnipeg
 drawing 198: America/Yakutat
 freq=199 composition code=198 for America/Yakutat
 drawing 199: America/Yellowknife
 freq=200 composition code=199 for America/Yellowknife
 drawing 200: Antarctica/Casey
 freq=201 composition code=200 for Antarctica/Casey
 drawing 201: Antarctica/Davis
 freq=202 composition code=201 for Antarctica/Davis
 drawing 202: Antarctica/DumontDUrville
 freq=203 composition code=202 for Antarctica/DumontDUrville
 drawing 203: Antarctica/Macquarie
 freq=204 composition code=203 for Antarctica/Macquarie
 drawing 204: Antarctica/Mawson
 freq=205 composition code=204 for Antarctica/Mawson
 drawing 205: Antarctica/McMurdo
 freq=206 composition code=205 for Antarctica/McMurdo
 drawing 206: Antarctica/Palmer
 freq=207 composition code=206 for Antarctica/Palmer
 drawing 207: Antarctica/Rothera
 freq=208 composition code=207 for Antarctica/Rothera
 drawing 208: Antarctica/Syowa
 freq=209 composition code=208 for Antarctica/Syowa
 drawing 209: Antarctica/Troll
 freq=210 composition code=209 for Antarctica/Troll
 drawing 210: Antarctica/Vostok
 freq=211 composition code=210 for Antarctica/Vostok
 drawing 211: Arctic/Longyearbyen
 freq=212 composition code=211 for Arctic/Longyearbyen
 drawing 212: Asia/Aden
 freq=213 composition code=212 for Asia/Aden
 drawing 213: Asia/Almaty
 freq=214 composition code=213 for Asia/Almaty
 drawing 214: Asia/Amman
 freq=215 composition code=214 for Asia/Amman
 drawing 215: Asia/Anadyr
 freq=216 composition code=215 for Asia/Anadyr
 drawing 216: Asia/Aqtau
 freq=217 composition code=216 for Asia/Aqtau
 drawing 217: Asia/Aqtobe
 freq=218 composition code=217 for Asia/Aqtobe
 drawing 218: Asia/Ashgabat
 freq=219 composition code=218 for Asia/Ashgabat
 drawing 219: Asia/Atyrau
 freq=220 composition code=219 for Asia/Atyrau
 drawing 220: Asia/Baghdad
 freq=221 composition code=220 for Asia/Baghdad
 drawing 221: Asia/Bahrain
 freq=222 composition code=221 for Asia/Bahrain
 drawing 222: Asia/Baku
 freq=223 composition code=222 for Asia/Baku
 drawing 223: Asia/Bangkok
 freq=224 composition code=223 for Asia/Bangkok
 drawing 224: Asia/Barnaul
 freq=225 composition code=224 for Asia/Barnaul
 drawing 225: Asia/Beirut
 freq=226 composition code=225 for Asia/Beirut
 drawing 226: Asia/Bishkek
 freq=227 composition code=226 for Asia/Bishkek
 drawing 227: Asia/Brunei
 freq=228 composition code=227 for Asia/Brunei
 drawing 228: Asia/Chita
 freq=229 composition code=228 for Asia/Chita
 drawing 229: Asia/Choibalsan
 freq=230 composition code=229 for Asia/Choibalsan
 drawing 230: Asia/Colombo
 freq=231 composition code=230 for Asia/Colombo
 drawing 231: Asia/Damascus
 freq=232 composition code=231 for Asia/Damascus
 drawing 232: Asia/Dhaka
 freq=233 composition code=232 for Asia/Dhaka
 drawing 233: Asia/Dili
 freq=234 composition code=233 for Asia/Dili
 drawing 234: Asia/Dubai
 freq=235 composition code=234 for Asia/Dubai
 drawing 235: Asia/Dushanbe
 freq=236 composition code=235 for Asia/Dushanbe
 drawing 236: Asia/Famagusta
 freq=237 composition code=236 for Asia/Famagusta
 drawing 237: Asia/Gaza
 freq=238 composition code=237 for Asia/Gaza
 drawing 238: Asia/Hebron
 freq=239 composition code=238 for Asia/Hebron
 drawing 239: Asia/Ho_Chi_Minh
 freq=240 composition code=239 for Asia/Ho_Chi_Minh
 drawing 240: Asia/Hong_Kong
 freq=241 composition code=240 for Asia/Hong_Kong
 drawing 241: Asia/Hovd
 freq=242 composition code=241 for Asia/Hovd
 drawing 242: Asia/Irkutsk
 freq=243 composition code=242 for Asia/Irkutsk
 drawing 243: Asia/Jakarta
 freq=244 composition code=243 for Asia/Jakarta
 drawing 244: Asia/Jayapura
 freq=245 composition code=244 for Asia/Jayapura
 drawing 245: Asia/Jerusalem
 freq=246 composition code=245 for Asia/Jerusalem
 drawing 246: Asia/Kabul
 freq=247 composition code=246 for Asia/Kabul
 drawing 247: Asia/Kamchatka
 freq=248 composition code=247 for Asia/Kamchatka
 drawing 248: Asia/Karachi
 freq=249 composition code=248 for Asia/Karachi
 drawing 249: Asia/Kathmandu
 freq=250 composition code=249 for Asia/Kathmandu
 drawing 250: Asia/Khandyga
 freq=251 composition code=250 for Asia/Khandyga
 drawing 251: Asia/Kolkata
 freq=252 composition code=251 for Asia/Kolkata
 drawing 252: Asia/Krasnoyarsk
 freq=253 composition code=252 for Asia/Krasnoyarsk
 drawing 253: Asia/Kuala_Lumpur
 freq=254 composition code=253 for Asia/Kuala_Lumpur
 drawing 254: Asia/Kuching
 freq=255 composition code=254 for Asia/Kuching
 drawing 255: Asia/Kuwait
 freq=256 composition code=255 for Asia/Kuwait
 drawing 256: Asia/Macau
 freq=257 composition code=256 for Asia/Macau
 drawing 257: Asia/Magadan
 freq=258 composition code=257 for Asia/Magadan
 drawing 258: Asia/Makassar
 freq=259 composition code=258 for Asia/Makassar
 drawing 259: Asia/Manila
 freq=260 composition code=259 for Asia/Manila
 drawing 260: Asia/Muscat
 freq=261 composition code=260 for Asia/Muscat
 drawing 261: Asia/Nicosia
 freq=262 composition code=261 for Asia/Nicosia
 drawing 262: Asia/Novokuznetsk
 freq=263 composition code=262 for Asia/Novokuznetsk
 drawing 263: Asia/Novosibirsk
 freq=264 composition code=263 for Asia/Novosibirsk
 drawing 264: Asia/Omsk
 freq=265 composition code=264 for Asia/Omsk
 drawing 265: Asia/Oral
 freq=266 composition code=265 for Asia/Oral
 drawing 266: Asia/Phnom_Penh
 freq=267 composition code=266 for Asia/Phnom_Penh
 drawing 267: Asia/Pontianak
 freq=268 composition code=267 for Asia/Pontianak
 drawing 268: Asia/Pyongyang
 freq=269 composition code=268 for Asia/Pyongyang
 drawing 269: Asia/Qatar
 freq=270 composition code=269 for Asia/Qatar
 drawing 270: Asia/Qostanay
 freq=271 composition code=270 for Asia/Qostanay
 drawing 271: Asia/Qyzylorda
 freq=272 composition code=271 for Asia/Qyzylorda
 drawing 272: Asia/Riyadh
 freq=273 composition code=272 for Asia/Riyadh
 drawing 273: Asia/Samarkand
 freq=274 composition code=273 for Asia/Samarkand
 drawing 274: Asia/Sakhalin
 freq=275 composition code=274 for Asia/Sakhalin
 drawing 275: Asia/Seoul
 freq=276 composition code=275 for Asia/Seoul
 drawing 276: Asia/Shanghai
 freq=277 composition code=276 for Asia/Shanghai
 drawing 277: Asia/Singapore
 freq=278 composition code=277 for Asia/Singapore
 drawing 278: Asia/Srednekolymsk
 freq=279 composition code=278 for Asia/Srednekolymsk
 drawing 279: Asia/Taipei
 freq=280 composition code=279 for Asia/Taipei
 drawing 280: Asia/Tashkent
 freq=281 composition code=280 for Asia/Tashkent
 drawing 281: Asia/Tbilisi
 freq=282 composition code=281 for Asia/Tbilisi
 drawing 282: Asia/Tehran
 freq=283 composition code=282 for Asia/Tehran
 drawing 283: Asia/Thimphu
 freq=284 composition code=283 for Asia/Thimphu
 drawing 284: Asia/Tokyo
 freq=285 composition code=284 for Asia/Tokyo
 drawing 285: Asia/Tomsk
 freq=286 composition code=285 for Asia/Tomsk
 drawing 286: Asia/Ulaanbaatar
 freq=287 composition code=286 for Asia/Ulaanbaatar
 drawing 287: Asia/Urumqi
 freq=288 composition code=287 for Asia/Urumqi
 drawing 288: Asia/Ust-Nera
 freq=289 composition code=288 for Asia/Ust-Nera
 drawing 289: Asia/Vientiane
 freq=290 composition code=289 for Asia/Vientiane
 drawing 290: Asia/Vladivostok
 freq=291 composition code=290 for Asia/Vladivostok
 drawing 291: Asia/Yakutsk
 freq=292 composition code=291 for Asia/Yakutsk
 drawing 292: Asia/Yangon
 freq=293 composition code=292 for Asia/Yangon
 drawing 293: Asia/Yekaterinburg
 freq=294 composition code=293 for Asia/Yekaterinburg
 drawing 294: Asia/Yerevan
 freq=295 composition code=294 for Asia/Yerevan
 drawing 295: Atlantic/Azores
 freq=296 composition code=295 for Atlantic/Azores
 drawing 296: Atlantic/Bermuda
 freq=297 composition code=296 for Atlantic/Bermuda
 drawing 297: Atlantic/Canary
 freq=298 composition code=297 for Atlantic/Canary
 drawing 298: Atlantic/Cape_Verde
 freq=299 composition code=298 for Atlantic/Cape_Verde
 drawing 299: Atlantic/Faroe
 freq=300 composition code=299 for Atlantic/Faroe
 drawing 300: Atlantic/Madeira
 freq=301 composition code=300 for Atlantic/Madeira
 drawing 301: Atlantic/Reykjavik
 freq=302 composition code=301 for Atlantic/Reykjavik
 drawing 302: Atlantic/South_Georgia
 freq=303 composition code=302 for Atlantic/South_Georgia
 drawing 303: Atlantic/St_Helena
 freq=304 composition code=303 for Atlantic/St_Helena
 drawing 304: Atlantic/Stanley
 freq=305 composition code=304 for Atlantic/Stanley
 drawing 305: Australia/Adelaide
 freq=306 composition code=305 for Australia/Adelaide
 drawing 306: Australia/Brisbane
 freq=307 composition code=306 for Australia/Brisbane
 drawing 307: Australia/Broken_Hill
 freq=308 composition code=307 for Australia/Broken_Hill
 drawing 308: Australia/Currie
 freq=309 composition code=308 for Australia/Currie
 drawing 309: Australia/Darwin
 freq=310 composition code=309 for Australia/Darwin
 drawing 310: Australia/Eucla
 freq=311 composition code=310 for Australia/Eucla
 drawing 311: Australia/Hobart
 freq=312 composition code=311 for Australia/Hobart
 drawing 312: Australia/Lindeman
 freq=313 composition code=312 for Australia/Lindeman
 drawing 313: Australia/Lord_Howe
 freq=314 composition code=313 for Australia/Lord_Howe
 drawing 314: Australia/Melbourne
 freq=315 composition code=314 for Australia/Melbourne
 drawing 315: Australia/Perth
 freq=316 composition code=315 for Australia/Perth
 drawing 316: Australia/Sydney
 freq=317 composition code=316 for Australia/Sydney
 drawing 317: Etc/UTC
 freq=318 composition code=317 for Etc/UTC
 drawing 318: Europe/Amsterdam
 freq=319 composition code=318 for Europe/Amsterdam
 drawing 319: Europe/Andorra
 freq=320 composition code=319 for Europe/Andorra
 drawing 320: Europe/Astrakhan
 freq=321 composition code=320 for Europe/Astrakhan
 drawing 321: Europe/Athens
 freq=322 composition code=321 for Europe/Athens
 drawing 322: Europe/Belgrade
 freq=323 composition code=322 for Europe/Belgrade
 drawing 323: Europe/Berlin
 freq=324 composition code=323 for Europe/Berlin
 drawing 324: Europe/Bratislava
 freq=325 composition code=324 for Europe/Bratislava
 drawing 325: Europe/Brussels
 freq=326 composition code=325 for Europe/Brussels
 drawing 326: Europe/Bucharest
 freq=327 composition code=326 for Europe/Bucharest
 drawing 327: Europe/Budapest
 freq=328 composition code=327 for Europe/Budapest
 drawing 328: Europe/Busingen
 freq=329 composition code=328 for Europe/Busingen
 drawing 329: Europe/Chisinau
 freq=330 composition code=329 for Europe/Chisinau
 drawing 330: Europe/Copenhagen
 freq=331 composition code=330 for Europe/Copenhagen
 drawing 331: Europe/Dublin
 freq=332 composition code=331 for Europe/Dublin
 drawing 332: Europe/Gibraltar
 freq=333 composition code=332 for Europe/Gibraltar
 drawing 333: Europe/Guernsey
 freq=334 composition code=333 for Europe/Guernsey
 drawing 334: Europe/Helsinki
 freq=335 composition code=334 for Europe/Helsinki
 drawing 335: Europe/Isle_of_Man
 freq=336 composition code=335 for Europe/Isle_of_Man
 drawing 336: Europe/Istanbul
 freq=337 composition code=336 for Europe/Istanbul
 drawing 337: Europe/Jersey
 freq=338 composition code=337 for Europe/Jersey
 drawing 338: Europe/Kaliningrad
 freq=339 composition code=338 for Europe/Kaliningrad
 drawing 339: Europe/Kiev
 freq=340 composition code=339 for Europe/Kiev
 drawing 340: Europe/Kirov
 freq=341 composition code=340 for Europe/Kirov
 drawing 341: Europe/Lisbon
 freq=342 composition code=341 for Europe/Lisbon
 drawing 342: Europe/Ljubljana
 freq=343 composition code=342 for Europe/Ljubljana
 drawing 343: Europe/London
 freq=344 composition code=343 for Europe/London
 drawing 344: Europe/Luxembourg
 freq=345 composition code=344 for Europe/Luxembourg
 drawing 345: Europe/Madrid
 freq=346 composition code=345 for Europe/Madrid
 drawing 346: Europe/Malta
 freq=347 composition code=346 for Europe/Malta
 drawing 347: Europe/Mariehamn
 freq=348 composition code=347 for Europe/Mariehamn
 drawing 348: Europe/Minsk
 freq=349 composition code=348 for Europe/Minsk
 drawing 349: Europe/Monaco
 freq=350 composition code=349 for Europe/Monaco
 drawing 350: Europe/Moscow
 freq=351 composition code=350 for Europe/Moscow
 drawing 351: Europe/Oslo
 freq=352 composition code=351 for Europe/Oslo
 drawing 352: Europe/Paris
 freq=353 composition code=352 for Europe/Paris
 drawing 353: Europe/Podgorica
 freq=354 composition code=353 for Europe/Podgorica
 drawing 354: Europe/Prague
 freq=355 composition code=354 for Europe/Prague
 drawing 355: Europe/Riga
 freq=356 composition code=355 for Europe/Riga
 drawing 356: Europe/Rome
 freq=357 composition code=356 for Europe/Rome
 drawing 357: Europe/Samara
 freq=358 composition code=357 for Europe/Samara
 drawing 358: Europe/San_Marino
 freq=359 composition code=358 for Europe/San_Marino
 drawing 359: Europe/Sarajevo
 freq=360 composition code=359 for Europe/Sarajevo
 drawing 360: Europe/Saratov
 freq=361 composition code=360 for Europe/Saratov
 drawing 361: Europe/Simferopol
 freq=362 composition code=361 for Europe/Simferopol
 drawing 362: Europe/Skopje
 freq=363 composition code=362 for Europe/Skopje
 drawing 363: Europe/Sofia
 freq=364 composition code=363 for Europe/Sofia
 drawing 364: Europe/Stockholm
 freq=365 composition code=364 for Europe/Stockholm
 drawing 365: Europe/Tallinn
 freq=366 composition code=365 for Europe/Tallinn
 drawing 366: Europe/Tirane
 freq=367 composition code=366 for Europe/Tirane
 drawing 367: Europe/Ulyanovsk
 freq=368 composition code=367 for Europe/Ulyanovsk
 drawing 368: Europe/Uzhgorod
 freq=369 composition code=368 for Europe/Uzhgorod
 drawing 369: Europe/Vaduz
 freq=370 composition code=369 for Europe/Vaduz
 drawing 370: Europe/Vatican
 freq=371 composition code=370 for Europe/Vatican
 drawing 371: Europe/Vienna
 freq=372 composition code=371 for Europe/Vienna
 drawing 372: Europe/Vilnius
 freq=373 composition code=372 for Europe/Vilnius
 drawing 373: Europe/Volgograd
 freq=374 composition code=373 for Europe/Volgograd
 drawing 374: Europe/Warsaw
 freq=375 composition code=374 for Europe/Warsaw
 drawing 375: Europe/Zagreb
 freq=376 composition code=375 for Europe/Zagreb
 drawing 376: Europe/Zaporozhye
 freq=377 composition code=376 for Europe/Zaporozhye
 drawing 377: Europe/Zurich
 freq=378 composition code=377 for Europe/Zurich
 drawing 378: Indian/Antananarivo
 freq=379 composition code=378 for Indian/Antananarivo
 drawing 379: Indian/Chagos
 freq=380 composition code=379 for Indian/Chagos
 drawing 380: Indian/Christmas
 freq=381 composition code=380 for Indian/Christmas
 drawing 381: Indian/Cocos
 freq=382 composition code=381 for Indian/Cocos
 drawing 382: Indian/Comoro
 freq=383 composition code=382 for Indian/Comoro
 drawing 383: Indian/Kerguelen
 freq=384 composition code=383 for Indian/Kerguelen
 drawing 384: Indian/Mahe
 freq=385 composition code=384 for Indian/Mahe
 drawing 385: Indian/Maldives
 freq=386 composition code=385 for Indian/Maldives
 drawing 386: Indian/Mauritius
 freq=387 composition code=386 for Indian/Mauritius
 drawing 387: Indian/Mayotte
 freq=388 composition code=387 for Indian/Mayotte
 drawing 388: Indian/Reunion
 freq=389 composition code=388 for Indian/Reunion
 drawing 389: Pacific/Apia
 freq=390 composition code=389 for Pacific/Apia
 drawing 390: Pacific/Auckland
 freq=391 composition code=390 for Pacific/Auckland
 drawing 391: Pacific/Bougainville
 freq=392 composition code=391 for Pacific/Bougainville
 drawing 392: Pacific/Chatham
 freq=393 composition code=392 for Pacific/Chatham
 drawing 393: Pacific/Chuuk
 freq=394 composition code=393 for Pacific/Chuuk
 drawing 394: Pacific/Easter
 freq=395 composition code=394 for Pacific/Easter
 drawing 395: Pacific/Efate
 freq=396 composition code=395 for Pacific/Efate
 drawing 396: Pacific/Enderbury
 freq=397 composition code=396 for Pacific/Enderbury
 drawing 397: Pacific/Fakaofo
 freq=398 composition code=397 for Pacific/Fakaofo
 drawing 398: Pacific/Fiji
 freq=399 composition code=398 for Pacific/Fiji
 drawing 399: Pacific/Funafuti
 freq=400 composition code=399 for Pacific/Funafuti
 drawing 400: Pacific/Galapagos
 freq=401 composition code=400 for Pacific/Galapagos
 drawing 401: Pacific/Gambier
 freq=402 composition code=401 for Pacific/Gambier
 drawing 402: Pacific/Guadalcanal
 freq=403 composition code=402 for Pacific/Guadalcanal
 drawing 403: Pacific/Guam
 freq=404 composition code=403 for Pacific/Guam
 drawing 404: Pacific/Honolulu
 freq=405 composition code=404 for Pacific/Honolulu
 drawing 405: Pacific/Kiritimati
 freq=406 composition code=405 for Pacific/Kiritimati
 drawing 406: Pacific/Kosrae
 freq=407 composition code=406 for Pacific/Kosrae
 drawing 407: Pacific/Kwajalein
 freq=408 composition code=407 for Pacific/Kwajalein
 drawing 408: Pacific/Majuro
 freq=409 composition code=408 for Pacific/Majuro
 drawing 409: Pacific/Marquesas
 freq=410 composition code=409 for Pacific/Marquesas
 drawing 410: Pacific/Midway
 freq=411 composition code=410 for Pacific/Midway
 drawing 411: Pacific/Nauru
 freq=412 composition code=411 for Pacific/Nauru
 drawing 412: Pacific/Niue
 freq=413 composition code=412 for Pacific/Niue
 drawing 413: Pacific/Norfolk
 freq=414 composition code=413 for Pacific/Norfolk
 drawing 414: Pacific/Noumea
 freq=415 composition code=414 for Pacific/Noumea
 drawing 415: Pacific/Pago_Pago
 freq=416 composition code=415 for Pacific/Pago_Pago
 drawing 416: Pacific/Palau
 freq=417 composition code=416 for Pacific/Palau
 drawing 417: Pacific/Pitcairn
 freq=418 composition code=417 for Pacific/Pitcairn
 drawing 418: Pacific/Pohnpei
 freq=419 composition code=418 for Pacific/Pohnpei
 drawing 419: Pacific/Port_Moresby
 freq=420 composition code=419 for Pacific/Port_Moresby
 drawing 420: Pacific/Rarotonga
 freq=421 composition code=420 for Pacific/Rarotonga
 drawing 421: Pacific/Saipan
 freq=422 composition code=421 for Pacific/Saipan
 drawing 422: Pacific/Tahiti
 freq=423 composition code=422 for Pacific/Tahiti
 drawing 423: Pacific/Tarawa
 freq=424 composition code=423 for Pacific/Tarawa
 drawing 424: Pacific/Tongatapu
 freq=425 composition code=424 for Pacific/Tongatapu
 drawing 425: Pacific/Wake
 freq=426 composition code=425 for Pacific/Wake
 drawing 426: Pacific/Wallis
 freq=427 composition code=426 for Pacific/Wallis
File Size(file) = 960000000;
File Size(file) = 1920000000;
File Size(file) = 2880000000;
File Size(file) = 3840000000;

//:*/

composition = 0;

composition = J( 16000, 30000, 0 );

composition[1::4000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(0),readlength(960000000))),"float",8,"little",30000);

composition[4001::8000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(1*960000000),readlength(960000000))),"float",8,"little",30000);

composition[8001::12000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(2*960000000),readlength(960000000))),"float",8,"little",30000);

composition[12001::16000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(3*960000000),readlength(960000000))),"float",8,"little",30000);

42;
/*:

42
//:*/

img = 0;

img = New Image( Heat Color( composition / Max( composition ), "spectral" ) );

img << scale( 1 / 16 ); // make it fit screen better. maybe use 1/8 on a 4K display.

New Window( "x", img );


/*:

DisplayBox[EvalContextBox]
//:*/
	codecounts = [=>0];

	rle = {};

	for( irow = 1, irow <= N Rows( composition ), irow += 1,

		if(mod(irow,100)==0,write(irow," ");wait(.01));

		row = {}; // a list for one row

		code = -1; // initial no match

		length = 0; // initial no length

		for( icol = 1, icol <= N Cols( composition ), icol += 1,

			if( composition[irow, icol] != code, // new run begins

				if( length > 0, // initial no match has length 0

					run = {}; // output code&length of prev run

					codecounts[code]+=1;

					run[1] = code;

					run[2] = length;

					row[N Items( row ) + 1] = run;

				);

				length = 1;

				code = composition[irow, icol];//

			, // else

				length += 1

			)

		);

		run = {};// handle the last one on the row

		codecounts[code]+=1;

		run[1] = code;

		run[2] = length;

		row[N Items( row ) + 1] = run;

		rle[N Items( rle ) + 1] = Matrix( row );// convert to Nx2 matrix and store at end of rle list

	);



	countkeys = codecounts<<getkeys; // 0..426. these are iFeature values.

	countvals = codecounts<<getvalues; // {108243, 849, 653, 1253, ... }

	rnk = rank(countvals);

	countvals = reverse(countvals[rnk]);

	countkeys = reverse(countkeys[rnk]); // these are iFeatures in big to small order

	if(countkeys[1]!=0,throw("countkeys"));

	freqCodeToName = iFeatureToName[countkeys[2::nitems(countkeys)]];

	insertinto(freqCodeToName,"water",1); // 427

	/*

	save for another run if nothing above changed

	*/

	Save Text File( "$desktop/rletext.jsl", Char( rle ) );

	Save Text File( "$desktop/rlename.jsl", char(freqCodeToName) );
/*:
100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 9100 9200 9300 9400 9500 9600 9700 9800 9900 10000 10100 10200 10300 10400 10500 10600 10700 10800 10900 11000 11100 11200 11300 11400 11500 11600 11700 11800 11900 12000 12100 12200 12300 12400 12500 12600 12700 12800 12900 13000 13100 13200 13300 13400 13500 13600 13700 13800 13900 14000 14100 14200 14300 14400 14500 14600 14700 14800 14900 15000 15100 15200 15300 15400 15500 15600 15700 15800 15900 16000 
"C:\Users\v1\Desktop\rlename.jsl"
//:*/

RleTableIndex = J( N Items( rle ), 1, . );

RleTableDataBlob = Char To Blob( "" );

address = 0; // increments according to compression and writes to RleTableIndex

irun = 0;

partBlob = Char To Blob( "" );

for( i = 1, i <= N Items( rle ), i += 1,

	if(mod(i,100)==0,write(i," ");wait(0););

	RleTableIndex[i] = address;

	k = 0;

	for( j = 1, j <= N Items( rle[i] ) / 2, j += 1,

		irun += 1;

		k += 1;

		code = rle[i][k];

		k += 1;

		length = rle[i][k];

		if(!(0<=code<=426),throw("code"||char(code)));

		if(!(1<=length<=30000),throw("length"||char(length)));

		partBlob = partBlob || (if( code <= 127,

			address += 1;

			Matrix To Blob( Matrix( code ), "int", 1, "big" ); // big: sign first

		,

			address += 2;

			Matrix To Blob( Matrix( -(code) ), "int", 2, "big" );

		) || if( length <= 127,

			address += 1;

			Matrix To Blob( Matrix( length ), "int", 1, "big" );

		,

			address += 2;

			Matrix To Blob( Matrix( -length ), "int", 2, "big" );

		));

	);

	if(mod(i,100)==0,RleTableDataBlob = RleTableDataBlob || partBlob;	partBlob = Char To Blob( "" ););

);

show(length(RleTableDataBlob));// 1,385,104 // 1,519,228 then 1,519,319 



/*:
100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 9100 9200 9300 9400 9500 9600 9700 9800 9900 10000 10100 10200 10300 10400 10500 10600 10700 10800 10900 11000 11100 11200 11300 11400 11500 11600 11700 11800 11900 12000 12100 12200 12300 12400 12500 12600 12700 12800 12900 13000 13100 13200 13300 13400 13500 13600 13700 13800 13900 14000 14100 14200 14300 14400 14500 14600 14700 14800 14900 15000 15100 15200 15300 15400 15500 15600 15700 15800 15900 16000 
Length(RleTableDataBlob) = 1518958;

//:*/

StringTableIndex = J( Nitems( iFeatureToName )+1, 1, . );

null = Hex To Blob( "00" );

StringTableDataBlob = null; // water. represented by a zero-length null terminated string.

StringTableIndex[1] = 0; // pointer to water

/*

at the very top, a table to convert from name to rule was loaded. make a lookup AA

and then translate the feature name to the rule and put the rule in the blob.

You won't care about the Nuuk/Godthab issue, I hope. It shows up here if Nuuk

is unknown by the gen-tz.py code mentioned earlier as key not found. A kludge

fix is shown, it might need more. My fix involved putting all the items in

iFeatureToName into gen-tz.py; that allowed it to lookup the rule in the linux system.

*/

xlate=associativearray(dtTrans:from<<getvalues,dtTrans:to<<getvalues); // rename col1=from, col2=to

// 'America/Nuuk' is 'America/Godthab'

//if(!xlate<<contains("America/Nuuk"),

//xlate["America/Nuuk"] = xlate["America/Godthab"];

//);

for( i = 1, i <= Nitems( iFeatureToName ), i += 1,

	StringTableIndex[i+1] = Length( StringTableDataBlob );

	tznamet=try(xlate[iFeatureToName[i]],"unknown");// "Asia/Qostanay" is new this time

	StringTableDataBlob = StringTableDataBlob || (chartoblob(tznamet) || null);

	//show(i,chartoblob(iFeatureToName[i]),length(StringTableDataBlob));

);

Length( StringTableDataBlob );//7022

/*:

Scoped data table access requires a data table column or variable in access or evaluation of 'dtTrans:from' , dtTrans:from/*###*/

at line 897 in C:\Users\v1\Documents\Script
//:*/
dtTrans
/*:

Data Table( "zones" )

// Change column name: Column 1 → from
Data Table( "zones" ):Column 1 << Set Name( "from" );


// Change column name: Column 2 → to
Data Table( "zones" ):Column 2 << Set Name( "to" );

//:*/

StringTableIndex = J( Nitems( iFeatureToName )+1, 1, . );

null = Hex To Blob( "00" );

StringTableDataBlob = null; // water. represented by a zero-length null terminated string.

StringTableIndex[1] = 0; // pointer to water

/*

at the very top, a table to convert from name to rule was loaded. make a lookup AA

and then translate the feature name to the rule and put the rule in the blob.

You won't care about the Nuuk/Godthab issue, I hope. It shows up here if Nuuk

is unknown by the gen-tz.py code mentioned earlier as key not found. A kludge

fix is shown, it might need more. My fix involved putting all the items in

iFeatureToName into gen-tz.py; that allowed it to lookup the rule in the linux system.

*/

xlate=associativearray(dtTrans:from<<getvalues,dtTrans:to<<getvalues); // rename col1=from, col2=to

// 'America/Nuuk' is 'America/Godthab'

//if(!xlate<<contains("America/Nuuk"),

//xlate["America/Nuuk"] = xlate["America/Godthab"];

//);

for( i = 1, i <= Nitems( iFeatureToName ), i += 1,

	StringTableIndex[i+1] = Length( StringTableDataBlob );

	tznamet=try(xlate[iFeatureToName[i]],"unknown");// "Asia/Qostanay" is new this time

	StringTableDataBlob = StringTableDataBlob || (chartoblob(tznamet) || null);

	//show(i,chartoblob(iFeatureToName[i]),length(StringTableDataBlob));

);

Length( StringTableDataBlob );//7022

/*:

5818
//:*/


stiLength = 4*nitems(StringTableIndex);// 4-byte offsets, string table's length

rtiLength = 4*nitems(RleTableIndex); // 4-byte offsets, run table's length

stdLength = length(StringTableDataBlob);

n=12;// number of 4-byte integers, not counting the first n value, counting the "END." value

// first build a matrix of 4-byte integers. some are offsets, some are values.

// 4+n*4 is the header length, including the initial n value.

header = n // number of offsets and ints that follow:

	||((4+n*4)+0) // offset to string index table

	||((4+n*4)+stiLength) // offset to string data "America/New_York\0" and similar packed strings

	||((4+n*4)+stiLength+stdLength) // offset to rle index table

	||((4+n*4)+stiLength+stdLength+rtiLength) // offset to rle data 1 or 2 compressed

	||((4+n*4)+stiLength+stdLength+rtiLength+length(RleTableDataBlob)) // date. sanity checking. not used for much.

	||nrows(composition) // lat

	||ncols(composition) // lon

	||-90||90 // lat coverage for nrows

	||-180||180 // lon cover for ncols

	||hextonumber(hex(reverse("END.")));// little endian reverses again (hextonumber data is big endian)

if(n!=nitems(header)-1,throw("header"));

/*

convert the header matrix to a blob

*/

headerBlob = matrixtoblob(header,"int",4,"little");

hbLength = length(headerBlob);



/*

Convert the string table index to a blob. NOTE the multiply by 0! these are not

pointers and not relocated offsets. they are relative offsets within the string table data

(distinguish between string table INDEX and string table DATA; the first item in the

INDEX is the offset of the first string in the DATA.)

*/

stiBlob = matrixToBlob(StringTableIndex+0*(hbLength+stiLength/*relocate ptrs*/),"int",4,"little");// NOT relocated, index offsets relative to StringTableDataBlob

if(length(stiBlob)!=stiLength,throw("stiBlob"));



/* ditto the RLE table INDEX and DATA */

rtiBlob = matrixtoblob(RleTableIndex+0*(hbLength+stiLength+rtiLength/*relocate ptrs*/),"int",4,"little");// NOT relocated,... RleTableDataBlob

if(length(rtiBlob)!=rtiLength,throw("rtiBlob"));



/* put it together, in order */

finalblob = headerBlob 

			|| stiBlob 

			|| StringTableDataBlob 

			|| rtiBlob 

			|| RleTableDataBlob 

			|| chartoblob("TZ lookup built "||char(asdate(today()))) || null;



write("\!nfinalblob size=",length(finalblob));// pass1=1590571, pass2=1590932


/*:

finalblob size=1590571

//:*/


// a little testing, not much 

write(

	"\!nstring NY=",

	blobpeek(finalblob,

		blobtomatrix( 

			Blob Peek( finalblob, 

				Blob To Matrix( Blob Peek( finalblob, 1/*string index*/ * 4, 4 ), "int", 4, "little" )[1]/*offset to index*/ + 4 * 152/*offset in index to offset of NY in string data*/

				, 4 

			),

			"int",4,"little"

		)

		+

		Blob To Matrix( Blob Peek( finalblob, 2/*data*/ * 4, 4 ), "int", 4, "little" )[1]/*offset to start of string data*/

		,

		17

	)

);

/*:

string NY=Char To Blob( "EST5EDT,M3.2.0,M1", "ascii~hex" )

//:*/

write("\!nsignature=",

	blobpeek(

		finalblob,

		Blob To Matrix( Blob Peek( finalblob, 5 * 4, 4 ), "int", 4, "little" )[1],

		300

	)

);

/*:

signature=Char To Blob( "TZ lookup built 20Oct2021:21:34:25~00", "ascii~hex" )

//:*/

lookup = function({lat=41.91,lon},{row,col,r,c,code},

	row=floor(interpolate(lat,-90,N Rows( composition ),90,1+0))-0;

	r=RleTableIndex[row];

	col=round(interpolate(lon,-180,1+0,180,N Cols( composition )))-0;

	c=0;

	while(c<col,

		code = blobtomatrix(blobpeek(RleTableDataBlob,r,1),"int",1,"big")[1];

		if(code<0,

			code = -blobtomatrix(blobpeek(RleTableDataBlob,r,2),"int",2,"big")[1];

			r+=2;//

		,//

			r+=1;

		);

		length = blobtomatrix(blobpeek(RleTableDataBlob,r,1),"int",1,"big")[1];

		if(length<0,

			length = -blobtomatrix(blobpeek(RleTableDataBlob,r,2),"int",2,"big")[1];

			r+=2;//

		,//

			r+=1;

		);

		c += length;

	);

	if(code>0,

		iFeatureToName[code];

	,//

		"water"

	)

);
/*:

Function( {lat = 41.91, lon},
    {row, col, r, c, code},
    row = Floor( Interpolate( lat, -90, N Rows( composition ), 90, 1 + 0 ) ) - 0;
    r = RleTableIndex[row];
    col = Round( Interpolate( lon, -180, 1 + 0, 180, N Cols( composition ) ) ) - 0;
    c = 0;
    While( c < col,
        code = Blob To Matrix(
            Blob Peek( RleTableDataBlob, r, 1 ),
            "int",
            1,
            "big"
        )[1];
        If( code < 0,
            code = -Blob To Matrix(
                Blob Peek( RleTableDataBlob, r, 2 ),
                "int",
                2,
                "big"
            )[1];
            r += 2;
        ,
            r += 1
        );
        length = Blob To Matrix(
            Blob Peek( RleTableDataBlob, r, 1 ),
            "int",
            1,
            "big"
        )[1];
        If( length < 0,
            length = -Blob To Matrix(
                Blob Peek( RleTableDataBlob, r, 2 ),
                "int",
                2,
                "big"
            )[1];
            r += 2;
        ,
            r += 1
        );
        c += length;
    );
    If( code > 0,
        iFeatureToName[code],
        "water"
    );
)
//:*/

markersOver = function({clat,clon,name,grid=1},

	dtVat=New Table( name,

		New Column( "tz", Character, "Nominal" ),

		New Column( "lat", Numeric, "Continuous", Format( "Best", 12 ) ),

		New Column( "lon", Numeric, "Continuous", Format( "Best", 12 ) )

	);



	for(ylat=clat-.5*grid, ylat<=clat+.5*grid,ylat+=.004*grid,

		for(xlon=clon-.5*grid,xlon<=clon+.5*grid,xlon+=.006*grid,

			dtVat<<addrows(1);

			dtVat:tz=lookup(ylat,xlon);

			dtVat:lat=ylat;

			dtVat:lon=xlon;

		)

	);

	dtVat:tz<<label(1);

	dtVat<< Color or Mark by Column(dtVat:tz);

	dtVat<<setdirty(0);

	dtVat = Graph Builder(

		Size( 1843, 976 ),

		Show Control Panel( 0 ),

		Variables( X( :lon ), Y( :lat ) ),

		Elements( Points( X, Y, Legend( 3 ) ) ),

		SendToReport(

//			Dispatch(

//				{},

//				"lon",

//				ScaleBox,

//				{Min( 12.4031559352746 ), Max( 12.5218440647255 ), Inc( 0.01 ),

//				Minor Ticks( 1 )}

//			),

//			Dispatch(

//				{},

//				"lat",

//				ScaleBox,

//				{Scale( "Geodesic" ), Format( "Best", 12 ), Min( 41.8801686871589 ),

//				Max( 41.9397483777795 ), Inc( 0.01 ), Minor Ticks( 4 )}

//			),

			Dispatch(

				{},

				"Graph Builder",

				FrameBox,

				//{Background Map( Images( "Street Map Service" ) ), Marker Size( 6 )}

				{Background Map(

					Images(

						"Web Map Service", "https://ows.terrestris.de/osm-gray/service?",

						"OSM-WMS"

					)

				), Marker Size( 1 ), Marker Drawing Mode( "Normal" )}

			)

		)

	);

);

/*

the smallest timezone *does* align part way under the data. The pixels in the data are bigger than 

this timezone and it falls on the edge of a pixel.

Kansas is interesting because of its western counties.

*/

markersover(41.905,12.445,"vatican city");
/*:

filesnapper watchdog callback

Graph Builder[]

// Report snapshot: vatican city - Graph Builder
Data Table( "vatican city" ) << Graph Builder(
    Size( 1825, 905 ),
    Show Control Panel( 0 ),
    Variables( X( :lon ), Y( :lat ) ),
    Elements( Points( X, Y, Legend( 3 ) ) ),
    SendToReport(
        Dispatch(
            {},
            "Graph Builder",
            FrameBox,
            {Background Map(
                Images(
                    "Web Map Service", "https://ows.terrestris.de/osm-gray/service?",
                    "OSM-WMS"
                )
            ), Marker Size( 1 ), Marker Drawing Mode( "Normal" )}
        )
    )
);


// Close Data Table: vatican city
Close( "vatican city", NoSave );

filesnapper watchdog callback

//:*/
markersover(38.76012940186213, -85.07358907248562, "America/Indiana/Vevay") ;
/*:

filesnapper watchdog callback

Graph Builder[]

// Report snapshot: America-Indiana-Vevay - Graph Builder
Data Table( "America-Indiana-Vevay" ) <<
Graph Builder(
    Size( 1825, 905 ),
    Show Control Panel( 0 ),
    Variables( X( :lon ), Y( :lat ) ),
    Elements( Points( X, Y, Legend( 3 ) ) ),
    SendToReport(
        Dispatch(
            {},
            "Graph Builder",
            FrameBox,
            {Background Map(
                Images(
                    "Web Map Service", "https://ows.terrestris.de/osm-gray/service?",
                    "OSM-WMS"
                )
            ), Marker Size( 1 ), Marker Drawing Mode( "Normal" )}
        )
    )
);


// Close Data Table: America-Indiana-Vevay
Close( "America-Indiana-Vevay", NoSave );

filesnapper watchdog callback

//:*/
P = Eval( Parse( Load Text File( "$desktop/rlejson.jsl" ) ) ); // loads faster this way
/*:

Associative array( 2 elements ) assigned.

//:*/
featureList = P["features"]; // list of 426 AAs (no water)
/*:

List( 426 elements ) assigned.

//:*/
rle = Eval( Parse( Load Text File( "$desktop/rletext.jsl" ) ) );
/*:

List( 16000 elements ) assigned.

//:*/
freqCodeToName = Eval( Parse( Load Text File( "$desktop/rlename.jsl" ) ) );
/*:

List( 421 elements ) assigned.

//:*/

iFeatureToName = {};

for( global:iFeature = 1, global:iFeature <= N Items( featureList ), global:iFeature += 1,

	tzdata = featureList[global:iFeature]; // aa {"geometry", "properties", "type"}

	tzproperties = tzdata["properties"]; // aa ["tzid" => "Africa/Abidjan"]

	tzname=tzproperties["tzid"];

	insertinto(iFeatureToName,tzname);

	if(!contains(freqCodeToName,tzname),

		write("\!nadd ",tzname);

		insertinto(freqCodeToName, tzname); // see comment above about "freqCodeToName={"water"};" -- this makes the uncompressed version

	);

);
/*:

add America/Metlakatla
add Asia/Hebron
add Asia/Hong_Kong
add Asia/Macau
add Europe/Busingen
add Europe/Gibraltar

//:*/

if( freqCodeToName[1] != "water" | freqCodeToName[2] != "Asia/Shanghai" | freqCodeToName[426] != "Europe/Busingen" | freqCodeToName[427] != "Europe/Vatican",

	Throw( "freqCodeToName" ) // probably comment this out on the first run which won't be sorted, and maybe later if the world changes.

);
/*:

freqCodeToName at row 41917

//:*/
freqCodeToName[1]
/*:

"water"
//:*/
freqCodeToName[2]
/*:

"Asia/Shanghai"
//:*/
freqCodeToName[426]
/*:

"Europe/Busingen"
//:*/
 freqCodeToName[427]
/*:

"Europe/Gibraltar"
//:*/
freqCodeToName
/*:

{"water", "Asia/Shanghai", "Europe/Moscow", "Asia/Kolkata", "Asia/Vladivostok",
"Asia/Krasnoyarsk", "Australia/Perth", "Africa/Johannesburg", "America/Iqaluit",
"America/Toronto", "Asia/Yakutsk", "Asia/Tokyo", "Antarctica/McMurdo",
"Australia/Brisbane", "America/Santiago", "Antarctica/Rothera", "Asia/Yekaterinburg",
"Pacific/Tahiti", "Asia/Bangkok", "America/Rankin_Inlet", "America/Chicago",
"America/Argentina/Ushuaia", "America/Sao_Paulo", "America/Bogota",
"America/New_York", "America/Lima", "Asia/Yangon", "Africa/Lubumbashi",
"America/Nipigon", "America/Nuuk", "Asia/Irkutsk", "Asia/Almaty",
"America/Anchorage", "Asia/Makassar", "America/Nome", "America/Denver",
"Antarctica/Vostok", "Antarctica/Syowa", "Antarctica/Mawson", "Asia/Anadyr",
"Asia/Urumqi", "Africa/Maputo", "Pacific/Auckland", "Antarctica/Davis",
"Asia/Tehran", "Africa/Bamako", "America/Los_Angeles", "America/Manaus",
"Asia/Jayapura", "Asia/Jakarta", "Africa/Algiers", "Africa/Khartoum", "Asia/Manila",
"America/Inuvik", "Europe/Rome", "Asia/Kamchatka", "Asia/Karachi",
"Australia/Darwin", "Antarctica/Troll", "Africa/Kinshasa", "Africa/Tripoli",
"America/Yellowknife", "America/La_Paz", "Asia/Riyadh", "Africa/Ndjamena",
"America/Argentina/Cordoba", "Europe/Kiev", "Africa/Luanda", "America/Caracas",
"America/Argentina/Salta", "America/Vancouver", "Etc/UTC", "Asia/Srednekolymsk",
"Europe/Stockholm", "Europe/Oslo", "America/Belem", "Pacific/Majuro",
"America/Mexico_City", "Pacific/Port_Moresby", "America/Cambridge_Bay",
"Europe/London", "Africa/Lusaka", "Africa/Mogadishu", "America/Pangnirtung",
"America/Goose_Bay", "Africa/Windhoek", "Europe/Paris", "Africa/Niamey",
"America/Bahia", "America/Monterrey", "America/Santarem", "Africa/Nouakchott",
"America/Mazatlan", "America/Fortaleza", "Australia/Adelaide", "Asia/Kabul",
"Africa/Addis_Ababa", "Indian/Antananarivo", "Europe/Madrid", "Africa/Brazzaville",
"Asia/Chita", "Africa/Douala", "Africa/Bangui", "Asia/Vientiane", "Europe/Istanbul",
"Pacific/Chuuk", "Africa/Lagos", "America/Cuiaba", "Asia/Ulaanbaatar",
"Australia/Sydney", "Africa/Juba", "Pacific/Guadalcanal", "Africa/Dar_es_Salaam",
"Asia/Magadan", "Europe/Helsinki", "Africa/Cairo", "America/Winnipeg",
"Asia/Ashgabat", "America/Edmonton", "Europe/Berlin", "Pacific/Noumea",
"Asia/Samarkand", "Africa/Gaborone", "Europe/Athens", "America/Swift_Current",
"America/Whitehorse", "America/Araguaina", "America/Regina", "Pacific/Fiji",
"Africa/Nairobi", "Asia/Sakhalin", "Asia/Ho_Chi_Minh", "Asia/Dhaka",
"America/Punta_Arenas", "Asia/Aden", "America/Argentina/Catamarca", "Asia/Muscat",
"Asia/Kuching", "Africa/Conakry", "Asia/Dushanbe", "America/Dawson", "Asia/Barnaul",
"America/Asuncion", "Asia/Baghdad", "Asia/Bishkek", "Asia/Ust-Nera", "Asia/Hovd",
"Asia/Pontianak", "Asia/Aqtobe", "Africa/Casablanca", "Europe/Samara",
"Asia/Khandyga", "Africa/Abidjan", "America/Boa_Vista", "America/Campo_Grande",
"Arctic/Longyearbyen", "Africa/Blantyre", "Africa/Libreville", "America/Matamoros",
"America/Guyana", "Europe/Kirov", "Asia/Qostanay", "Europe/Warsaw",
"Africa/Ouagadougou", "Europe/Minsk", "America/Argentina/Buenos_Aires",
"Pacific/Honolulu", "Indian/Mahe", "America/Porto_Velho", "America/Rainy_River",
"Australia/Melbourne", "Indian/Maldives", "America/St_Johns", "Asia/Kuala_Lumpur",
"Africa/Harare", "Asia/Phnom_Penh", "Asia/Omsk", "America/Chihuahua",
"Africa/Asmara", "America/Guayaquil", "Asia/Tomsk", "Africa/Tunis",
"America/Phoenix", "America/Adak", "Pacific/Efate", "Europe/Zagreb",
"Asia/Novosibirsk", "Asia/Baku", "Pacific/Tarawa", "America/Detroit",
"Asia/Tashkent", "Asia/Seoul", "Asia/Kathmandu", "Africa/Accra", "Asia/Pyongyang",
"Africa/Dakar", "Europe/Bucharest", "America/Argentina/Rio_Gallegos",
"America/Tegucigalpa", "Africa/El_Aaiun", "Asia/Choibalsan", "America/Hermosillo",
"America/Atikokan", "Africa/Kampala", "Africa/Porto-Novo", "Australia/Hobart",
"America/Recife", "America/Nassau", "Pacific/Tongatapu", "Pacific/Rarotonga",
"America/Merida", "America/Managua", "America/Montevideo", "Asia/Damascus",
"America/Argentina/Mendoza", "Europe/Lisbon", "Asia/Aqtau", "America/Eirunepe",
"America/Tijuana", "Europe/Saratov", "Asia/Novokuznetsk", "Asia/Qyzylorda",
"Asia/Taipei", "Antarctica/Casey", "America/Halifax", "Europe/Copenhagen",
"Europe/Belgrade", "Europe/Volgograd", "Asia/Atyrau", "Asia/Oral", "America/Boise",
"Africa/Monrovia", "America/Paramaribo", "America/Panama", "Europe/Vienna",
"Pacific/Saipan", "Africa/Lome", "Atlantic/Canary", "Asia/Dubai",
"Atlantic/South_Georgia", "America/Argentina/San_Juan", "America/Rio_Branco",
"America/Ojinaga", "Pacific/Bougainville", "Europe/Dublin", "Indian/Kerguelen",
"Europe/Prague", "Europe/Sofia", "Antarctica/DumontDUrville", "America/Cancun",
"Pacific/Funafuti", "America/Argentina/La_Rioja", "Atlantic/Reykjavik",
"America/Costa_Rica", "America/Moncton", "Asia/Amman", "Europe/Budapest",
"America/Guatemala", "America/Dawson_Creek", "Europe/Chisinau", "Europe/Zurich",
"Europe/Riga", "Europe/Astrakhan", "America/Thule", "America/Argentina/San_Luis",
"Europe/Ulyanovsk", "America/Havana", "Pacific/Pohnpei", "America/Cayenne",
"Asia/Colombo", "America/Santo_Domingo", "America/Sitka", "America/Argentina/Jujuy",
"Asia/Jerusalem", "Pacific/Kiritimati", "Europe/Amsterdam", "Asia/Tbilisi",
"Europe/Sarajevo", "Europe/Vilnius", "America/Maceio", "Pacific/Marquesas",
"Atlantic/Azores", "Africa/Freetown", "Europe/Bratislava", "Pacific/Enderbury",
"America/Blanc-Sablon", "Europe/Tallinn", "Europe/Brussels", "Pacific/Galapagos",
"America/Indiana/Indianapolis", "Asia/Yerevan", "Africa/Malabo",
"America/Port-au-Prince", "America/Juneau", "Europe/Tirane", "Indian/Mauritius",
"America/Belize", "America/Glace_Bay", "Atlantic/Cape_Verde", "America/Danmarkshavn",
"Asia/Qatar", "Africa/Bujumbura", "Europe/Zaporozhye", "Europe/Ljubljana",
"Africa/Kigali", "Pacific/Gambier", "Indian/Chagos", "Asia/Thimphu",
"America/Argentina/Tucuman", "Europe/Simferopol", "America/Fort_Nelson",
"Africa/Maseru", "Africa/Bissau", "Atlantic/Stanley", "Atlantic/St_Helena",
"Europe/Podgorica", "Africa/Djibouti", "America/Scoresbysund", "Africa/Sao_Tome",
"America/Jamaica", "Europe/Skopje", "Pacific/Palau", "Indian/Comoro", "Asia/Beirut",
"America/Menominee", "America/El_Salvador", "Asia/Nicosia", "Europe/Uzhgorod",
"America/Yakutat", "Asia/Brunei", "Atlantic/Madeira", "America/Puerto_Rico",
"Asia/Dili", "Africa/Mbabane", "America/Port_of_Spain", "Asia/Kuwait",
"America/Antigua", "Pacific/Pago_Pago", "Pacific/Fakaofo", "Europe/Mariehamn",
"Africa/Banjul", "Australia/Eucla", "Pacific/Chatham", "Asia/Famagusta",
"America/Kralendijk", "Europe/Kaliningrad", "Antarctica/Palmer",
"America/Grand_Turk", "America/Guadeloupe", "Antarctica/Macquarie", "Atlantic/Faroe",
"Pacific/Wallis", "America/St_Thomas", "Pacific/Kwajalein", "Asia/Bahrain",
"Pacific/Apia", "Australia/Currie", "America/St_Vincent", "America/Resolute",
"America/Cayman", "America/Thunder_Bay", "America/Creston",
"America/Indiana/Vincennes", "Indian/Reunion", "Australia/Broken_Hill",
"Indian/Mayotte", "Pacific/Midway", "America/Martinique", "America/Tortola",
"Pacific/Guam", "America/Curacao", "Europe/Luxembourg", "America/Miquelon",
"America/Grenada", "Indian/Cocos", "America/Anguilla", "America/Dominica",
"America/St_Lucia", "America/North_Dakota/New_Salem", "America/St_Kitts",
"Europe/Guernsey", "Australia/Lord_Howe", "Europe/Isle_of_Man",
"America/North_Dakota/Beulah", "America/Barbados", "Europe/Malta",
"America/Kentucky/Louisville", "America/Bahia_Banderas", "Pacific/Niue",
"Australia/Lindeman", "Pacific/Pitcairn", "Asia/Singapore", "Indian/Christmas",
"Europe/Jersey", "America/Aruba", "Pacific/Norfolk", "Pacific/Easter",
"Atlantic/Bermuda", "America/Montserrat", "Pacific/Kosrae", "Asia/Gaza",
"America/St_Barthelemy", "Pacific/Wake", "Pacific/Nauru",
"America/Indiana/Tell_City", "America/Kentucky/Monticello", "America/Noronha",
"America/Indiana/Petersburg", "America/Marigot", "America/North_Dakota/Center",
"America/Indiana/Marengo", "America/Lower_Princes", "America/Indiana/Winamac",
"America/Indiana/Knox", "America/Indiana/Vevay", "Europe/Andorra", "Europe/Monaco",
"Europe/Vaduz", "Africa/Ceuta", "Europe/San_Marino", "Europe/Vatican",
"America/Metlakatla", "Asia/Hebron", "Asia/Hong_Kong", "Asia/Macau",
"Europe/Busingen", "Europe/Gibraltar"}
//:*/

composition = 0;

composition = J( 16000, 30000, 0 );42;
/*:

42
//:*/

	for( ifreq = 2/* 1 is water */, ifreq <= N Items( freqCodeToName ), ifreq += 1,

		/*

		this is the big-to-small list of timezones; get the name of the zone to do next...

		*/

		thisname=freqCodeToName[ifreq];

		/*

		this global variable tells the poly routine what to work on; locate the name and use the index from loc

		*/

		global:iFeature = loc(iFeatureToName,thisname)[1];// draw smallest last

		/*

		Surprisingly nothing happens when the graphbox is made without a window, until...

		*/

		gb = Graph Box(

			X Scale( -180, 180 ),

			Y Scale( -90, 90 ),

			framesize( 30000 + 1, 16000 + 1 ), // drawing beyond 30,000 may not always work, 16x30 is about as big as it can be.

			<<backgroundcolor( "black" ),

			drawpolys();

			//marker(colorstate("green"),{-180,-90},{180,-90},{-180,90},{180,90},{0,0}); // corner markers might help figure out trimming, below

		);

		/*

		...the picture/bitmap is requested. That takes some time.

		*/

		pixels = gb[framebox( 1 )] << getpicture; // 30 seconds to make the picture

		gb = 0;

		pixels = pixels << getpixels; // 10 seconds to convert to pixel matrix

		/*

		trimming the image. added 1 in the graphbox to get the data inside a 30000x16000 

		(not 29999x15999) space, now strip off the extra frame pixels

		*/

		pixels = pixels[2 :: (N Rows( pixels ) - 5), 2 :: (N Cols( pixels ) - 5)];

		/*

		after another small sanity check, find the nonzero (rendered white timezone on black background)

		pixels and add them to the composition as *** global:iFeature *** value.

		iFeature will become the RLE code in the next step.

		*/

		if( N Rows( composition ) == N Rows( pixels ) & N Cols( composition ) == N Cols( pixels ),

			composition[Loc( pixels )] = global:iFeature; // 

		, // else

			Throw( "composition" )

		);

		write("\!n freq=", ifreq," composition code=",global:iFeature," for ",thisname );

		if(thisname == "America/New_York", write(" ***"));

		Wait( .1 );

	);

	/*

	delete the old file saving the composition, then recreate it.

	My computer was unable to do it in one save, so I split it 

	into four appends. Probably no real point in the delete since

	the first save is a replace.

	And this hardcodes the 16000 height, but that isn't the dimension

	I wish was bigger; the horizontal resolution is more important for

	timezones, most of the time.

	*/

	try(deletefile("$desktop/rlecomp.jsl"));



	bcomp = matrixtoblob(composition[1::4000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("replace") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[4001::8000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[8001::12000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));

	bcomp = matrixtoblob(composition[12001::16000,0],"float",8,"little");

	file=Save Text File( "$desktop/rlecomp.jsl", bcomp, mode("append") );

	show(filesize(file));
/*:

 drawing 276: Asia/Shanghai
 freq=2 composition code=276 for Asia/Shanghai
 drawing 350: Europe/Moscow
 freq=3 composition code=350 for Europe/Moscow
 drawing 251: Asia/Kolkata
 freq=4 composition code=251 for Asia/Kolkata
 drawing 290: Asia/Vladivostok
 freq=5 composition code=290 for Asia/Vladivostok
 drawing 252: Asia/Krasnoyarsk
 freq=6 composition code=252 for Asia/Krasnoyarsk
 drawing 315: Australia/Perth
 freq=7 composition code=315 for Australia/Perth
 drawing 25: Africa/Johannesburg
 freq=8 composition code=25 for Africa/Johannesburg
 drawing 125: America/Iqaluit
 freq=9 composition code=125 for America/Iqaluit
 drawing 193: America/Toronto
 freq=10 composition code=193 for America/Toronto
 drawing 291: Asia/Yakutsk
 freq=11 composition code=291 for Asia/Yakutsk
 drawing 284: Asia/Tokyo
 freq=12 composition code=284 for Asia/Tokyo
 drawing 205: Antarctica/McMurdo
 freq=13 composition code=205 for Antarctica/McMurdo
 drawing 306: Australia/Brisbane
 freq=14 composition code=306 for Australia/Brisbane
 drawing 177: America/Santiago
 freq=15 composition code=177 for America/Santiago
 drawing 207: Antarctica/Rothera
 freq=16 composition code=207 for Antarctica/Rothera
 drawing 293: Asia/Yekaterinburg
 freq=17 composition code=293 for Asia/Yekaterinburg
 drawing 422: Pacific/Tahiti
 freq=18 composition code=422 for Pacific/Tahiti
 drawing 223: Asia/Bangkok
 freq=19 composition code=223 for Asia/Bangkok
 drawing 171: America/Rankin_Inlet
 freq=20 composition code=171 for America/Rankin_Inlet
 drawing 88: America/Chicago
 freq=21 composition code=88 for America/Chicago
 drawing 70: America/Argentina/Ushuaia
 freq=22 composition code=70 for America/Argentina/Ushuaia
 drawing 179: America/Sao_Paulo
 freq=23 composition code=179 for America/Sao_Paulo
 drawing 80: America/Bogota
 freq=24 composition code=80 for America/Bogota
 drawing 152: America/New_York
 freq=25 composition code=152 for America/New_York ***
 drawing 132: America/Lima
 freq=26 composition code=132 for America/Lima
 drawing 292: Asia/Yangon
 freq=27 composition code=292 for Asia/Yangon
 drawing 35: Africa/Lubumbashi
 freq=28 composition code=35 for Africa/Lubumbashi
 drawing 153: America/Nipigon
 freq=29 composition code=153 for America/Nipigon
 drawing 159: America/Nuuk
 freq=30 composition code=159 for America/Nuuk
 drawing 242: Asia/Irkutsk
 freq=31 composition code=242 for Asia/Irkutsk
 drawing 213: Asia/Almaty
 freq=32 composition code=213 for Asia/Almaty
 drawing 54: America/Anchorage
 freq=33 composition code=54 for America/Anchorage
 drawing 258: Asia/Makassar
 freq=34 composition code=258 for Asia/Makassar
 drawing 154: America/Nome
 freq=35 composition code=154 for America/Nome
 drawing 97: America/Denver
 freq=36 composition code=97 for America/Denver
 drawing 210: Antarctica/Vostok
 freq=37 composition code=210 for Antarctica/Vostok
 drawing 208: Antarctica/Syowa
 freq=38 composition code=208 for Antarctica/Syowa
 drawing 204: Antarctica/Mawson
 freq=39 composition code=204 for Antarctica/Mawson
 drawing 215: Asia/Anadyr
 freq=40 composition code=215 for Asia/Anadyr
 drawing 287: Asia/Urumqi
 freq=41 composition code=287 for Asia/Urumqi
 drawing 38: Africa/Maputo
 freq=42 composition code=38 for Africa/Maputo
 drawing 390: Pacific/Auckland
 freq=43 composition code=390 for Pacific/Auckland
 drawing 201: Antarctica/Davis
 freq=44 composition code=201 for Antarctica/Davis
 drawing 282: Asia/Tehran
 freq=45 composition code=282 for Asia/Tehran
 drawing 6: Africa/Bamako
 freq=46 composition code=6 for Africa/Bamako
 drawing 133: America/Los_Angeles
 freq=47 composition code=133 for America/Los_Angeles
 drawing 137: America/Manaus
 freq=48 composition code=137 for America/Manaus
 drawing 244: Asia/Jayapura
 freq=49 composition code=244 for Asia/Jayapura
 drawing 243: Asia/Jakarta
 freq=50 composition code=243 for Asia/Jakarta
 drawing 4: Africa/Algiers
 freq=51 composition code=4 for Africa/Algiers
 drawing 28: Africa/Khartoum
 freq=52 composition code=28 for Africa/Khartoum
 drawing 259: Asia/Manila
 freq=53 composition code=259 for Asia/Manila
 drawing 124: America/Inuvik
 freq=54 composition code=124 for America/Inuvik
 drawing 356: Europe/Rome
 freq=55 composition code=356 for Europe/Rome
 drawing 247: Asia/Kamchatka
 freq=56 composition code=247 for Asia/Kamchatka
 drawing 248: Asia/Karachi
 freq=57 composition code=248 for Asia/Karachi
 drawing 309: Australia/Darwin
 freq=58 composition code=309 for Australia/Darwin
 drawing 209: Antarctica/Troll
 freq=59 composition code=209 for Antarctica/Troll
 drawing 30: Africa/Kinshasa
 freq=60 composition code=30 for Africa/Kinshasa
 drawing 50: Africa/Tripoli
 freq=61 composition code=50 for Africa/Tripoli
 drawing 199: America/Yellowknife
 freq=62 composition code=199 for America/Yellowknife
 drawing 131: America/La_Paz
 freq=63 composition code=131 for America/La_Paz
 drawing 272: Asia/Riyadh
 freq=64 composition code=272 for Asia/Riyadh
 drawing 44: Africa/Ndjamena
 freq=65 composition code=44 for Africa/Ndjamena
 drawing 61: America/Argentina/Cordoba
 freq=66 composition code=61 for America/Argentina/Cordoba
 drawing 339: Europe/Kiev
 freq=67 composition code=339 for Europe/Kiev
 drawing 34: Africa/Luanda
 freq=68 composition code=34 for Africa/Luanda
 drawing 85: America/Caracas
 freq=69 composition code=85 for America/Caracas
 drawing 66: America/Argentina/Salta
 freq=70 composition code=66 for America/Argentina/Salta
 drawing 195: America/Vancouver
 freq=71 composition code=195 for America/Vancouver
 drawing 317: Etc/UTC
 freq=72 composition code=317 for Etc/UTC
 drawing 278: Asia/Srednekolymsk
 freq=73 composition code=278 for Asia/Srednekolymsk
 drawing 364: Europe/Stockholm
 freq=74 composition code=364 for Europe/Stockholm
 drawing 351: Europe/Oslo
 freq=75 composition code=351 for Europe/Oslo
 drawing 76: America/Belem
 freq=76 composition code=76 for America/Belem
 drawing 408: Pacific/Majuro
 freq=77 composition code=408 for Pacific/Majuro
 drawing 146: America/Mexico_City
 freq=78 composition code=146 for America/Mexico_City
 drawing 419: Pacific/Port_Moresby
 freq=79 composition code=419 for Pacific/Port_Moresby
 drawing 82: America/Cambridge_Bay
 freq=80 composition code=82 for America/Cambridge_Bay
 drawing 343: Europe/London
 freq=81 composition code=343 for Europe/London
 drawing 36: Africa/Lusaka
 freq=82 composition code=36 for Africa/Lusaka
 drawing 41: Africa/Mogadishu
 freq=83 composition code=41 for Africa/Mogadishu
 drawing 162: America/Pangnirtung
 freq=84 composition code=162 for America/Pangnirtung
 drawing 106: America/Goose_Bay
 freq=85 composition code=106 for America/Goose_Bay
 drawing 52: Africa/Windhoek
 freq=86 composition code=52 for Africa/Windhoek
 drawing 352: Europe/Paris
 freq=87 composition code=352 for Europe/Paris
 drawing 45: Africa/Niamey
 freq=88 composition code=45 for Africa/Niamey
 drawing 73: America/Bahia
 freq=89 composition code=73 for America/Bahia
 drawing 148: America/Monterrey
 freq=90 composition code=148 for America/Monterrey
 drawing 176: America/Santarem
 freq=91 composition code=176 for America/Santarem
 drawing 46: Africa/Nouakchott
 freq=92 composition code=46 for Africa/Nouakchott
 drawing 141: America/Mazatlan
 freq=93 composition code=141 for America/Mazatlan
 drawing 104: America/Fortaleza
 freq=94 composition code=104 for America/Fortaleza
 drawing 305: Australia/Adelaide
 freq=95 composition code=305 for Australia/Adelaide
 drawing 246: Asia/Kabul
 freq=96 composition code=246 for Asia/Kabul
 drawing 3: Africa/Addis_Ababa
 freq=97 composition code=3 for Africa/Addis_Ababa
 drawing 378: Indian/Antananarivo
 freq=98 composition code=378 for Indian/Antananarivo
 drawing 345: Europe/Madrid
 freq=99 composition code=345 for Europe/Madrid
 drawing 11: Africa/Brazzaville
 freq=100 composition code=11 for Africa/Brazzaville
 drawing 228: Asia/Chita
 freq=101 composition code=228 for Asia/Chita
 drawing 20: Africa/Douala
 freq=102 composition code=20 for Africa/Douala
 drawing 7: Africa/Bangui
 freq=103 composition code=7 for Africa/Bangui
 drawing 289: Asia/Vientiane
 freq=104 composition code=289 for Asia/Vientiane
 drawing 336: Europe/Istanbul
 freq=105 composition code=336 for Europe/Istanbul
 drawing 393: Pacific/Chuuk
 freq=106 composition code=393 for Pacific/Chuuk
 drawing 31: Africa/Lagos
 freq=107 composition code=31 for Africa/Lagos
 drawing 92: America/Cuiaba
 freq=108 composition code=92 for America/Cuiaba
 drawing 286: Asia/Ulaanbaatar
 freq=109 composition code=286 for Asia/Ulaanbaatar
 drawing 316: Australia/Sydney
 freq=110 composition code=316 for Australia/Sydney
 drawing 26: Africa/Juba
 freq=111 composition code=26 for Africa/Juba
 drawing 402: Pacific/Guadalcanal
 freq=112 composition code=402 for Pacific/Guadalcanal
 drawing 18: Africa/Dar_es_Salaam
 freq=113 composition code=18 for Africa/Dar_es_Salaam
 drawing 257: Asia/Magadan
 freq=114 composition code=257 for Asia/Magadan
 drawing 334: Europe/Helsinki
 freq=115 composition code=334 for Europe/Helsinki
 drawing 13: Africa/Cairo
 freq=116 composition code=13 for Africa/Cairo
 drawing 197: America/Winnipeg
 freq=117 composition code=197 for America/Winnipeg
 drawing 218: Asia/Ashgabat
 freq=118 composition code=218 for Asia/Ashgabat
 drawing 100: America/Edmonton
 freq=119 composition code=100 for America/Edmonton
 drawing 323: Europe/Berlin
 freq=120 composition code=323 for Europe/Berlin
 drawing 414: Pacific/Noumea
 freq=121 composition code=414 for Pacific/Noumea
 drawing 273: Asia/Samarkand
 freq=122 composition code=273 for Asia/Samarkand
 drawing 23: Africa/Gaborone
 freq=123 composition code=23 for Africa/Gaborone
 drawing 321: Europe/Athens
 freq=124 composition code=321 for Europe/Athens
 drawing 188: America/Swift_Current
 freq=125 composition code=188 for America/Swift_Current
 drawing 196: America/Whitehorse
 freq=126 composition code=196 for America/Whitehorse
 drawing 58: America/Araguaina
 freq=127 composition code=58 for America/Araguaina
 drawing 173: America/Regina
 freq=128 composition code=173 for America/Regina
 drawing 398: Pacific/Fiji
 freq=129 composition code=398 for Pacific/Fiji
 drawing 43: Africa/Nairobi
 freq=130 composition code=43 for Africa/Nairobi
 drawing 274: Asia/Sakhalin
 freq=131 composition code=274 for Asia/Sakhalin
 drawing 239: Asia/Ho_Chi_Minh
 freq=132 composition code=239 for Asia/Ho_Chi_Minh
 drawing 232: Asia/Dhaka
 freq=133 composition code=232 for Asia/Dhaka
 drawing 169: America/Punta_Arenas
 freq=134 composition code=169 for America/Punta_Arenas
 drawing 212: Asia/Aden
 freq=135 composition code=212 for Asia/Aden
 drawing 60: America/Argentina/Catamarca
 freq=136 composition code=60 for America/Argentina/Catamarca
 drawing 260: Asia/Muscat
 freq=137 composition code=260 for Asia/Muscat
 drawing 254: Asia/Kuching
 freq=138 composition code=254 for Asia/Kuching
 drawing 16: Africa/Conakry
 freq=139 composition code=16 for Africa/Conakry
 drawing 235: Asia/Dushanbe
 freq=140 composition code=235 for Asia/Dushanbe
 drawing 95: America/Dawson
 freq=141 composition code=95 for America/Dawson
 drawing 224: Asia/Barnaul
 freq=142 composition code=224 for Asia/Barnaul
 drawing 71: America/Asuncion
 freq=143 composition code=71 for America/Asuncion
 drawing 220: Asia/Baghdad
 freq=144 composition code=220 for Asia/Baghdad
 drawing 226: Asia/Bishkek
 freq=145 composition code=226 for Asia/Bishkek
 drawing 288: Asia/Ust-Nera
 freq=146 composition code=288 for Asia/Ust-Nera
 drawing 241: Asia/Hovd
 freq=147 composition code=241 for Asia/Hovd
 drawing 267: Asia/Pontianak
 freq=148 composition code=267 for Asia/Pontianak
 drawing 217: Asia/Aqtobe
 freq=149 composition code=217 for Asia/Aqtobe
 drawing 14: Africa/Casablanca
 freq=150 composition code=14 for Africa/Casablanca
 drawing 357: Europe/Samara
 freq=151 composition code=357 for Europe/Samara
 drawing 250: Asia/Khandyga
 freq=152 composition code=250 for Asia/Khandyga
 drawing 1: Africa/Abidjan
 freq=153 composition code=1 for Africa/Abidjan
 drawing 79: America/Boa_Vista
 freq=154 composition code=79 for America/Boa_Vista
 drawing 83: America/Campo_Grande
 freq=155 composition code=83 for America/Campo_Grande
 drawing 211: Arctic/Longyearbyen
 freq=156 composition code=211 for Arctic/Longyearbyen
 drawing 10: Africa/Blantyre
 freq=157 composition code=10 for Africa/Blantyre
 drawing 32: Africa/Libreville
 freq=158 composition code=32 for Africa/Libreville
 drawing 140: America/Matamoros
 freq=159 composition code=140 for America/Matamoros
 drawing 112: America/Guyana
 freq=160 composition code=112 for America/Guyana
 drawing 340: Europe/Kirov
 freq=161 composition code=340 for Europe/Kirov
 drawing 270: Asia/Qostanay
 freq=162 composition code=270 for Asia/Qostanay
 drawing 374: Europe/Warsaw
 freq=163 composition code=374 for Europe/Warsaw
 drawing 47: Africa/Ouagadougou
 freq=164 composition code=47 for Africa/Ouagadougou
 drawing 348: Europe/Minsk
 freq=165 composition code=348 for Europe/Minsk
 drawing 59: America/Argentina/Buenos_Aires
 freq=166 composition code=59 for America/Argentina/Buenos_Aires
 drawing 404: Pacific/Honolulu
 freq=167 composition code=404 for Pacific/Honolulu
 drawing 384: Indian/Mahe
 freq=168 composition code=384 for Indian/Mahe
 drawing 167: America/Porto_Velho
 freq=169 composition code=167 for America/Porto_Velho
 drawing 170: America/Rainy_River
 freq=170 composition code=170 for America/Rainy_River
 drawing 314: Australia/Melbourne
 freq=171 composition code=314 for Australia/Melbourne
 drawing 385: Indian/Maldives
 freq=172 composition code=385 for Indian/Maldives
 drawing 183: America/St_Johns
 freq=173 composition code=183 for America/St_Johns
 drawing 253: Asia/Kuala_Lumpur
 freq=174 composition code=253 for Asia/Kuala_Lumpur
 drawing 24: Africa/Harare
 freq=175 composition code=24 for Africa/Harare
 drawing 266: Asia/Phnom_Penh
 freq=176 composition code=266 for Asia/Phnom_Penh
 drawing 264: Asia/Omsk
 freq=177 composition code=264 for Asia/Omsk
 drawing 89: America/Chihuahua
 freq=178 composition code=89 for America/Chihuahua
 drawing 5: Africa/Asmara
 freq=179 composition code=5 for Africa/Asmara
 drawing 111: America/Guayaquil
 freq=180 composition code=111 for America/Guayaquil
 drawing 285: Asia/Tomsk
 freq=181 composition code=285 for Asia/Tomsk
 drawing 51: Africa/Tunis
 freq=182 composition code=51 for Africa/Tunis
 drawing 164: America/Phoenix
 freq=183 composition code=164 for America/Phoenix
 drawing 53: America/Adak
 freq=184 composition code=53 for America/Adak
 drawing 395: Pacific/Efate
 freq=185 composition code=395 for Pacific/Efate
 drawing 375: Europe/Zagreb
 freq=186 composition code=375 for Europe/Zagreb
 drawing 263: Asia/Novosibirsk
 freq=187 composition code=263 for Asia/Novosibirsk
 drawing 222: Asia/Baku
 freq=188 composition code=222 for Asia/Baku
 drawing 423: Pacific/Tarawa
 freq=189 composition code=423 for Pacific/Tarawa
 drawing 98: America/Detroit
 freq=190 composition code=98 for America/Detroit
 drawing 280: Asia/Tashkent
 freq=191 composition code=280 for Asia/Tashkent
 drawing 275: Asia/Seoul
 freq=192 composition code=275 for Asia/Seoul
 drawing 249: Asia/Kathmandu
 freq=193 composition code=249 for Asia/Kathmandu
 drawing 2: Africa/Accra
 freq=194 composition code=2 for Africa/Accra
 drawing 268: Asia/Pyongyang
 freq=195 composition code=268 for Asia/Pyongyang
 drawing 17: Africa/Dakar
 freq=196 composition code=17 for Africa/Dakar
 drawing 326: Europe/Bucharest
 freq=197 composition code=326 for Europe/Bucharest
 drawing 65: America/Argentina/Rio_Gallegos
 freq=198 composition code=65 for America/Argentina/Rio_Gallegos
 drawing 189: America/Tegucigalpa
 freq=199 composition code=189 for America/Tegucigalpa
 drawing 21: Africa/El_Aaiun
 freq=200 composition code=21 for Africa/El_Aaiun
 drawing 229: Asia/Choibalsan
 freq=201 composition code=229 for Asia/Choibalsan
 drawing 115: America/Hermosillo
 freq=202 composition code=115 for America/Hermosillo
 drawing 72: America/Atikokan
 freq=203 composition code=72 for America/Atikokan
 drawing 27: Africa/Kampala
 freq=204 composition code=27 for Africa/Kampala
 drawing 48: Africa/Porto-Novo
 freq=205 composition code=48 for Africa/Porto-Novo
 drawing 311: Australia/Hobart
 freq=206 composition code=311 for Australia/Hobart
 drawing 172: America/Recife
 freq=207 composition code=172 for America/Recife
 drawing 151: America/Nassau
 freq=208 composition code=151 for America/Nassau
 drawing 424: Pacific/Tongatapu
 freq=209 composition code=424 for Pacific/Tongatapu
 drawing 420: Pacific/Rarotonga
 freq=210 composition code=420 for Pacific/Rarotonga
 drawing 144: America/Merida
 freq=211 composition code=144 for America/Merida
 drawing 136: America/Managua
 freq=212 composition code=136 for America/Managua
 drawing 149: America/Montevideo
 freq=213 composition code=149 for America/Montevideo
 drawing 231: Asia/Damascus
 freq=214 composition code=231 for Asia/Damascus
 drawing 64: America/Argentina/Mendoza
 freq=215 composition code=64 for America/Argentina/Mendoza
 drawing 341: Europe/Lisbon
 freq=216 composition code=341 for Europe/Lisbon
 drawing 216: Asia/Aqtau
 freq=217 composition code=216 for Asia/Aqtau
 drawing 101: America/Eirunepe
 freq=218 composition code=101 for America/Eirunepe
 drawing 192: America/Tijuana
 freq=219 composition code=192 for America/Tijuana
 drawing 360: Europe/Saratov
 freq=220 composition code=360 for Europe/Saratov
 drawing 262: Asia/Novokuznetsk
 freq=221 composition code=262 for Asia/Novokuznetsk
 drawing 271: Asia/Qyzylorda
 freq=222 composition code=271 for Asia/Qyzylorda
 drawing 279: Asia/Taipei
 freq=223 composition code=279 for Asia/Taipei
 drawing 200: Antarctica/Casey
 freq=224 composition code=200 for Antarctica/Casey
 drawing 113: America/Halifax
 freq=225 composition code=113 for America/Halifax
 drawing 330: Europe/Copenhagen
 freq=226 composition code=330 for Europe/Copenhagen
 drawing 322: Europe/Belgrade
 freq=227 composition code=322 for Europe/Belgrade
 drawing 373: Europe/Volgograd
 freq=228 composition code=373 for Europe/Volgograd
 drawing 219: Asia/Atyrau
 freq=229 composition code=219 for Asia/Atyrau
 drawing 265: Asia/Oral
 freq=230 composition code=265 for Asia/Oral
 drawing 81: America/Boise
 freq=231 composition code=81 for America/Boise
 drawing 42: Africa/Monrovia
 freq=232 composition code=42 for Africa/Monrovia
 drawing 163: America/Paramaribo
 freq=233 composition code=163 for America/Paramaribo
 drawing 161: America/Panama
 freq=234 composition code=161 for America/Panama
 drawing 371: Europe/Vienna
 freq=235 composition code=371 for Europe/Vienna
 drawing 421: Pacific/Saipan
 freq=236 composition code=421 for Pacific/Saipan
 drawing 33: Africa/Lome
 freq=237 composition code=33 for Africa/Lome
 drawing 297: Atlantic/Canary
 freq=238 composition code=297 for Atlantic/Canary
 drawing 234: Asia/Dubai
 freq=239 composition code=234 for Asia/Dubai
 drawing 302: Atlantic/South_Georgia
 freq=240 composition code=302 for Atlantic/South_Georgia
 drawing 67: America/Argentina/San_Juan
 freq=241 composition code=67 for America/Argentina/San_Juan
 drawing 175: America/Rio_Branco
 freq=242 composition code=175 for America/Rio_Branco
 drawing 160: America/Ojinaga
 freq=243 composition code=160 for America/Ojinaga
 drawing 391: Pacific/Bougainville
 freq=244 composition code=391 for Pacific/Bougainville
 drawing 331: Europe/Dublin
 freq=245 composition code=331 for Europe/Dublin
 drawing 383: Indian/Kerguelen
 freq=246 composition code=383 for Indian/Kerguelen
 drawing 354: Europe/Prague
 freq=247 composition code=354 for Europe/Prague
 drawing 363: Europe/Sofia
 freq=248 composition code=363 for Europe/Sofia
 drawing 202: Antarctica/DumontDUrville
 freq=249 composition code=202 for Antarctica/DumontDUrville
 drawing 84: America/Cancun
 freq=250 composition code=84 for America/Cancun
 drawing 399: Pacific/Funafuti
 freq=251 composition code=399 for Pacific/Funafuti
 drawing 63: America/Argentina/La_Rioja
 freq=252 composition code=63 for America/Argentina/La_Rioja
 drawing 301: Atlantic/Reykjavik
 freq=253 composition code=301 for Atlantic/Reykjavik
 drawing 90: America/Costa_Rica
 freq=254 composition code=90 for America/Costa_Rica
 drawing 147: America/Moncton
 freq=255 composition code=147 for America/Moncton
 drawing 214: Asia/Amman
 freq=256 composition code=214 for Asia/Amman
 drawing 327: Europe/Budapest
 freq=257 composition code=327 for Europe/Budapest
 drawing 110: America/Guatemala
 freq=258 composition code=110 for America/Guatemala
 drawing 96: America/Dawson_Creek
 freq=259 composition code=96 for America/Dawson_Creek
 drawing 329: Europe/Chisinau
 freq=260 composition code=329 for Europe/Chisinau
 drawing 377: Europe/Zurich
 freq=261 composition code=377 for Europe/Zurich
 drawing 355: Europe/Riga
 freq=262 composition code=355 for Europe/Riga
 drawing 320: Europe/Astrakhan
 freq=263 composition code=320 for Europe/Astrakhan
 drawing 190: America/Thule
 freq=264 composition code=190 for America/Thule
 drawing 68: America/Argentina/San_Luis
 freq=265 composition code=68 for America/Argentina/San_Luis
 drawing 367: Europe/Ulyanovsk
 freq=266 composition code=367 for Europe/Ulyanovsk
 drawing 114: America/Havana
 freq=267 composition code=114 for America/Havana
 drawing 418: Pacific/Pohnpei
 freq=268 composition code=418 for Pacific/Pohnpei
 drawing 86: America/Cayenne
 freq=269 composition code=86 for America/Cayenne
 drawing 230: Asia/Colombo
 freq=270 composition code=230 for Asia/Colombo
 drawing 178: America/Santo_Domingo
 freq=271 composition code=178 for America/Santo_Domingo
 drawing 181: America/Sitka
 freq=272 composition code=181 for America/Sitka
 drawing 62: America/Argentina/Jujuy
 freq=273 composition code=62 for America/Argentina/Jujuy
 drawing 245: Asia/Jerusalem
 freq=274 composition code=245 for Asia/Jerusalem
 drawing 405: Pacific/Kiritimati
 freq=275 composition code=405 for Pacific/Kiritimati
 drawing 318: Europe/Amsterdam
 freq=276 composition code=318 for Europe/Amsterdam
 drawing 281: Asia/Tbilisi
 freq=277 composition code=281 for Asia/Tbilisi
 drawing 359: Europe/Sarajevo
 freq=278 composition code=359 for Europe/Sarajevo
 drawing 372: Europe/Vilnius
 freq=279 composition code=372 for Europe/Vilnius
 drawing 135: America/Maceio
 freq=280 composition code=135 for America/Maceio
 drawing 409: Pacific/Marquesas
 freq=281 composition code=409 for Pacific/Marquesas
 drawing 295: Atlantic/Azores
 freq=282 composition code=295 for Atlantic/Azores
 drawing 22: Africa/Freetown
 freq=283 composition code=22 for Africa/Freetown
 drawing 324: Europe/Bratislava
 freq=284 composition code=324 for Europe/Bratislava
 drawing 396: Pacific/Enderbury
 freq=285 composition code=396 for Pacific/Enderbury
 drawing 78: America/Blanc-Sablon
 freq=286 composition code=78 for America/Blanc-Sablon
 drawing 365: Europe/Tallinn
 freq=287 composition code=365 for Europe/Tallinn
 drawing 325: Europe/Brussels
 freq=288 composition code=325 for Europe/Brussels
 drawing 400: Pacific/Galapagos
 freq=289 composition code=400 for Pacific/Galapagos
 drawing 116: America/Indiana/Indianapolis
 freq=290 composition code=116 for America/Indiana/Indianapolis
 drawing 294: Asia/Yerevan
 freq=291 composition code=294 for Asia/Yerevan
 drawing 37: Africa/Malabo
 freq=292 composition code=37 for Africa/Malabo
 drawing 165: America/Port-au-Prince
 freq=293 composition code=165 for America/Port-au-Prince
 drawing 127: America/Juneau
 freq=294 composition code=127 for America/Juneau
 drawing 366: Europe/Tirane
 freq=295 composition code=366 for Europe/Tirane
 drawing 386: Indian/Mauritius
 freq=296 composition code=386 for Indian/Mauritius
 drawing 77: America/Belize
 freq=297 composition code=77 for America/Belize
 drawing 105: America/Glace_Bay
 freq=298 composition code=105 for America/Glace_Bay
 drawing 298: Atlantic/Cape_Verde
 freq=299 composition code=298 for Atlantic/Cape_Verde
 drawing 94: America/Danmarkshavn
 freq=300 composition code=94 for America/Danmarkshavn
 drawing 269: Asia/Qatar
 freq=301 composition code=269 for Asia/Qatar
 drawing 12: Africa/Bujumbura
 freq=302 composition code=12 for Africa/Bujumbura
 drawing 376: Europe/Zaporozhye
 freq=303 composition code=376 for Europe/Zaporozhye
 drawing 342: Europe/Ljubljana
 freq=304 composition code=342 for Europe/Ljubljana
 drawing 29: Africa/Kigali
 freq=305 composition code=29 for Africa/Kigali
 drawing 401: Pacific/Gambier
 freq=306 composition code=401 for Pacific/Gambier
 drawing 379: Indian/Chagos
 freq=307 composition code=379 for Indian/Chagos
 drawing 283: Asia/Thimphu
 freq=308 composition code=283 for Asia/Thimphu
 drawing 69: America/Argentina/Tucuman
 freq=309 composition code=69 for America/Argentina/Tucuman
 drawing 361: Europe/Simferopol
 freq=310 composition code=361 for Europe/Simferopol
 drawing 103: America/Fort_Nelson
 freq=311 composition code=103 for America/Fort_Nelson
 drawing 39: Africa/Maseru
 freq=312 composition code=39 for Africa/Maseru
 drawing 9: Africa/Bissau
 freq=313 composition code=9 for Africa/Bissau
 drawing 304: Atlantic/Stanley
 freq=314 composition code=304 for Atlantic/Stanley
 drawing 303: Atlantic/St_Helena
 freq=315 composition code=303 for Atlantic/St_Helena
 drawing 353: Europe/Podgorica
 freq=316 composition code=353 for Europe/Podgorica
 drawing 19: Africa/Djibouti
 freq=317 composition code=19 for Africa/Djibouti
 drawing 180: America/Scoresbysund
 freq=318 composition code=180 for America/Scoresbysund
 drawing 49: Africa/Sao_Tome
 freq=319 composition code=49 for Africa/Sao_Tome
 drawing 126: America/Jamaica
 freq=320 composition code=126 for America/Jamaica
 drawing 362: Europe/Skopje
 freq=321 composition code=362 for Europe/Skopje
 drawing 416: Pacific/Palau
 freq=322 composition code=416 for Pacific/Palau
 drawing 382: Indian/Comoro
 freq=323 composition code=382 for Indian/Comoro
 drawing 225: Asia/Beirut
 freq=324 composition code=225 for Asia/Beirut
 drawing 143: America/Menominee
 freq=325 composition code=143 for America/Menominee
 drawing 102: America/El_Salvador
 freq=326 composition code=102 for America/El_Salvador
 drawing 261: Asia/Nicosia
 freq=327 composition code=261 for Asia/Nicosia
 drawing 368: Europe/Uzhgorod
 freq=328 composition code=368 for Europe/Uzhgorod
 drawing 198: America/Yakutat
 freq=329 composition code=198 for America/Yakutat
 drawing 227: Asia/Brunei
 freq=330 composition code=227 for Asia/Brunei
 drawing 300: Atlantic/Madeira
 freq=331 composition code=300 for Atlantic/Madeira
 drawing 168: America/Puerto_Rico
 freq=332 composition code=168 for America/Puerto_Rico
 drawing 233: Asia/Dili
 freq=333 composition code=233 for Asia/Dili
 drawing 40: Africa/Mbabane
 freq=334 composition code=40 for Africa/Mbabane
 drawing 166: America/Port_of_Spain
 freq=335 composition code=166 for America/Port_of_Spain
 drawing 255: Asia/Kuwait
 freq=336 composition code=255 for Asia/Kuwait
 drawing 56: America/Antigua
 freq=337 composition code=56 for America/Antigua
 drawing 415: Pacific/Pago_Pago
 freq=338 composition code=415 for Pacific/Pago_Pago
 drawing 397: Pacific/Fakaofo
 freq=339 composition code=397 for Pacific/Fakaofo
 drawing 347: Europe/Mariehamn
 freq=340 composition code=347 for Europe/Mariehamn
 drawing 8: Africa/Banjul
 freq=341 composition code=8 for Africa/Banjul
 drawing 310: Australia/Eucla
 freq=342 composition code=310 for Australia/Eucla
 drawing 392: Pacific/Chatham
 freq=343 composition code=392 for Pacific/Chatham
 drawing 236: Asia/Famagusta
 freq=344 composition code=236 for Asia/Famagusta
 drawing 130: America/Kralendijk
 freq=345 composition code=130 for America/Kralendijk
 drawing 338: Europe/Kaliningrad
 freq=346 composition code=338 for Europe/Kaliningrad
 drawing 206: Antarctica/Palmer
 freq=347 composition code=206 for Antarctica/Palmer
 drawing 107: America/Grand_Turk
 freq=348 composition code=107 for America/Grand_Turk
 drawing 109: America/Guadeloupe
 freq=349 composition code=109 for America/Guadeloupe
 drawing 203: Antarctica/Macquarie
 freq=350 composition code=203 for Antarctica/Macquarie
 drawing 299: Atlantic/Faroe
 freq=351 composition code=299 for Atlantic/Faroe
 drawing 426: Pacific/Wallis
 freq=352 composition code=426 for Pacific/Wallis
 drawing 186: America/St_Thomas
 freq=353 composition code=186 for America/St_Thomas
 drawing 407: Pacific/Kwajalein
 freq=354 composition code=407 for Pacific/Kwajalein
 drawing 221: Asia/Bahrain
 freq=355 composition code=221 for Asia/Bahrain
 drawing 389: Pacific/Apia
 freq=356 composition code=389 for Pacific/Apia
 drawing 308: Australia/Currie
 freq=357 composition code=308 for Australia/Currie
 drawing 187: America/St_Vincent
 freq=358 composition code=187 for America/St_Vincent
 drawing 174: America/Resolute
 freq=359 composition code=174 for America/Resolute
 drawing 87: America/Cayman
 freq=360 composition code=87 for America/Cayman
 drawing 191: America/Thunder_Bay
 freq=361 composition code=191 for America/Thunder_Bay
 drawing 91: America/Creston
 freq=362 composition code=91 for America/Creston
 drawing 122: America/Indiana/Vincennes
 freq=363 composition code=122 for America/Indiana/Vincennes
 drawing 388: Indian/Reunion
 freq=364 composition code=388 for Indian/Reunion
 drawing 307: Australia/Broken_Hill
 freq=365 composition code=307 for Australia/Broken_Hill
 drawing 387: Indian/Mayotte
 freq=366 composition code=387 for Indian/Mayotte
 drawing 410: Pacific/Midway
 freq=367 composition code=410 for Pacific/Midway
 drawing 139: America/Martinique
 freq=368 composition code=139 for America/Martinique
 drawing 194: America/Tortola
 freq=369 composition code=194 for America/Tortola
 drawing 403: Pacific/Guam
 freq=370 composition code=403 for Pacific/Guam
 drawing 93: America/Curacao
 freq=371 composition code=93 for America/Curacao
 drawing 344: Europe/Luxembourg
 freq=372 composition code=344 for Europe/Luxembourg
 drawing 142: America/Miquelon
 freq=373 composition code=142 for America/Miquelon
 drawing 108: America/Grenada
 freq=374 composition code=108 for America/Grenada
 drawing 381: Indian/Cocos
 freq=375 composition code=381 for Indian/Cocos
 drawing 55: America/Anguilla
 freq=376 composition code=55 for America/Anguilla
 drawing 99: America/Dominica
 freq=377 composition code=99 for America/Dominica
 drawing 185: America/St_Lucia
 freq=378 composition code=185 for America/St_Lucia
 drawing 158: America/North_Dakota/New_Salem
 freq=379 composition code=158 for America/North_Dakota/New_Salem
 drawing 184: America/St_Kitts
 freq=380 composition code=184 for America/St_Kitts
 drawing 333: Europe/Guernsey
 freq=381 composition code=333 for Europe/Guernsey
 drawing 313: Australia/Lord_Howe
 freq=382 composition code=313 for Australia/Lord_Howe
 drawing 335: Europe/Isle_of_Man
 freq=383 composition code=335 for Europe/Isle_of_Man
 drawing 156: America/North_Dakota/Beulah
 freq=384 composition code=156 for America/North_Dakota/Beulah
 drawing 75: America/Barbados
 freq=385 composition code=75 for America/Barbados
 drawing 346: Europe/Malta
 freq=386 composition code=346 for Europe/Malta
 drawing 128: America/Kentucky/Louisville
 freq=387 composition code=128 for America/Kentucky/Louisville
 drawing 74: America/Bahia_Banderas
 freq=388 composition code=74 for America/Bahia_Banderas
 drawing 412: Pacific/Niue
 freq=389 composition code=412 for Pacific/Niue
 drawing 312: Australia/Lindeman
 freq=390 composition code=312 for Australia/Lindeman
 drawing 417: Pacific/Pitcairn
 freq=391 composition code=417 for Pacific/Pitcairn
 drawing 277: Asia/Singapore
 freq=392 composition code=277 for Asia/Singapore
 drawing 380: Indian/Christmas
 freq=393 composition code=380 for Indian/Christmas
 drawing 337: Europe/Jersey
 freq=394 composition code=337 for Europe/Jersey
 drawing 57: America/Aruba
 freq=395 composition code=57 for America/Aruba
 drawing 413: Pacific/Norfolk
 freq=396 composition code=413 for Pacific/Norfolk
 drawing 394: Pacific/Easter
 freq=397 composition code=394 for Pacific/Easter
 drawing 296: Atlantic/Bermuda
 freq=398 composition code=296 for Atlantic/Bermuda
 drawing 150: America/Montserrat
 freq=399 composition code=150 for America/Montserrat
 drawing 406: Pacific/Kosrae
 freq=400 composition code=406 for Pacific/Kosrae
 drawing 237: Asia/Gaza
 freq=401 composition code=237 for Asia/Gaza
 drawing 182: America/St_Barthelemy
 freq=402 composition code=182 for America/St_Barthelemy
 drawing 425: Pacific/Wake
 freq=403 composition code=425 for Pacific/Wake
 drawing 411: Pacific/Nauru
 freq=404 composition code=411 for Pacific/Nauru
 drawing 120: America/Indiana/Tell_City
 freq=405 composition code=120 for America/Indiana/Tell_City
 drawing 129: America/Kentucky/Monticello
 freq=406 composition code=129 for America/Kentucky/Monticello
 drawing 155: America/Noronha
 freq=407 composition code=155 for America/Noronha
 drawing 119: America/Indiana/Petersburg
 freq=408 composition code=119 for America/Indiana/Petersburg
 drawing 138: America/Marigot
 freq=409 composition code=138 for America/Marigot
 drawing 157: America/North_Dakota/Center
 freq=410 composition code=157 for America/North_Dakota/Center
 drawing 118: America/Indiana/Marengo
 freq=411 composition code=118 for America/Indiana/Marengo
 drawing 134: America/Lower_Princes
 freq=412 composition code=134 for America/Lower_Princes
 drawing 123: America/Indiana/Winamac
 freq=413 composition code=123 for America/Indiana/Winamac
 drawing 117: America/Indiana/Knox
 freq=414 composition code=117 for America/Indiana/Knox
 drawing 121: America/Indiana/Vevay
 freq=415 composition code=121 for America/Indiana/Vevay
 drawing 319: Europe/Andorra
 freq=416 composition code=319 for Europe/Andorra
 drawing 349: Europe/Monaco
 freq=417 composition code=349 for Europe/Monaco
 drawing 369: Europe/Vaduz
 freq=418 composition code=369 for Europe/Vaduz
 drawing 15: Africa/Ceuta
 freq=419 composition code=15 for Africa/Ceuta
 drawing 358: Europe/San_Marino
 freq=420 composition code=358 for Europe/San_Marino
 drawing 370: Europe/Vatican
 freq=421 composition code=370 for Europe/Vatican
 drawing 145: America/Metlakatla
 freq=422 composition code=145 for America/Metlakatla
 drawing 238: Asia/Hebron
 freq=423 composition code=238 for Asia/Hebron
 drawing 240: Asia/Hong_Kong
 freq=424 composition code=240 for Asia/Hong_Kong
 drawing 256: Asia/Macau
 freq=425 composition code=256 for Asia/Macau
 drawing 328: Europe/Busingen
 freq=426 composition code=328 for Europe/Busingen
 drawing 332: Europe/Gibraltar
 freq=427 composition code=332 for Europe/Gibraltar
File Size(file) = 960000000;
File Size(file) = 1920000000;
File Size(file) = 2880000000;
File Size(file) = 3840000000;

//:*/

composition = 0;

composition = J( 16000, 30000, 0 );

composition[1::4000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(0),readlength(960000000))),"float",8,"little",30000);

composition[4001::8000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(1*960000000),readlength(960000000))),"float",8,"little",30000);

composition[8001::12000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(2*960000000),readlength(960000000))),"float",8,"little",30000);

composition[12001::16000,0] = blobtomatrix(loadtextfile("$desktop/rlecomp.jsl",blob(readOffsetFromBegin(3*960000000),readlength(960000000))),"float",8,"little",30000);

42;


/*:

42
//:*/

img = 0;

img = New Image( Heat Color( composition / Max( composition ), "spectral" ) );

img << scale( 1 / 16 ); // make it fit screen better. maybe use 1/8 on a 4K display.

New Window( "x", img );



/*

This JSL mostly builds the RLE (run length encoded data), but as a side effect

counts how common the RLE codes are. 

This loop builds uncompressed pairs {code,length} in a list of Nx2-element matrices

rle is a 16000 element list; each element is a run length encoded row of the picture.

each row of rle is a Nx2 matrix of the rle encoded changes for the row.

*/


/*:

DisplayBox[EvalContextBox]
//:*/

	codecounts = [=>0];

	rle = {};

	for( irow = 1, irow <= N Rows( composition ), irow += 1,

		if(mod(irow,100)==0,write(irow," ");wait(.01));

		row = {}; // a list for one row

		code = -1; // initial no match

		length = 0; // initial no length

		for( icol = 1, icol <= N Cols( composition ), icol += 1,

			if( composition[irow, icol] != code, // new run begins

				if( length > 0, // initial no match has length 0

					run = {}; // output code&length of prev run

					codecounts[code]+=1;

					run[1] = code;

					run[2] = length;

					row[N Items( row ) + 1] = run;

				);

				length = 1;

				code = composition[irow, icol];//

			, // else

				length += 1

			)

		);

		run = {};// handle the last one on the row

		codecounts[code]+=1;

		run[1] = code;

		run[2] = length;

		row[N Items( row ) + 1] = run;

		rle[N Items( rle ) + 1] = Matrix( row );// convert to Nx2 matrix and store at end of rle list

	);



	countkeys = codecounts<<getkeys; // 0..426. these are iFeature values.

	countvals = codecounts<<getvalues; // {108243, 849, 653, 1253, ... }

	rnk = rank(countvals);

	countvals = reverse(countvals[rnk]);

	countkeys = reverse(countkeys[rnk]); // these are iFeatures in big to small order

	if(countkeys[1]!=0,throw("countkeys"));

	freqCodeToName = iFeatureToName[countkeys[2::nitems(countkeys)]];

	insertinto(freqCodeToName,"water",1); // 427

	/*

	save for another run if nothing above changed

	*/

	Save Text File( "$desktop/rletext.jsl", Char( rle ) );

	Save Text File( "$desktop/rlename.jsl", char(freqCodeToName) );


/*:
100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 9100 9200 9300 9400 9500 9600 9700 9800 9900 10000 10100 10200 10300 10400 10500 10600 10700 10800 10900 11000 11100 11200 11300 11400 11500 11600 11700 11800 11900 12000 12100 12200 12300 12400 12500 12600 12700 12800 12900 13000 13100 13200 13300 13400 13500 13600 13700 13800 13900 14000 14100 14200 14300 14400 14500 14600 14700 14800 14900 15000 15100 15200 15300 15400 15500 15600 15700 15800 15900 16000 
"C:\Users\v1\Desktop\rlename.jsl"
//:*/

RleTableIndex = J( N Items( rle ), 1, . );

RleTableDataBlob = Char To Blob( "" );

address = 0; // increments according to compression and writes to RleTableIndex

irun = 0;

partBlob = Char To Blob( "" );

for( i = 1, i <= N Items( rle ), i += 1,

	if(mod(i,100)==0,write(i," ");wait(0););

	RleTableIndex[i] = address;

	k = 0;

	for( j = 1, j <= N Items( rle[i] ) / 2, j += 1,

		irun += 1;

		k += 1;

		code = rle[i][k];

		k += 1;

		length = rle[i][k];

		if(!(0<=code<=426),throw("code"||char(code)));

		if(!(1<=length<=30000),throw("length"||char(length)));

		partBlob = partBlob || (if( code <= 127,

			address += 1;

			Matrix To Blob( Matrix( code ), "int", 1, "big" ); // big: sign first

		,

			address += 2;

			Matrix To Blob( Matrix( -(code) ), "int", 2, "big" );

		) || if( length <= 127,

			address += 1;

			Matrix To Blob( Matrix( length ), "int", 1, "big" );

		,

			address += 2;

			Matrix To Blob( Matrix( -length ), "int", 2, "big" );

		));

	);

	if(mod(i,100)==0,RleTableDataBlob = RleTableDataBlob || partBlob;	partBlob = Char To Blob( "" ););

);

show(length(RleTableDataBlob));// 1,385,104 // 1,519,228 then 1,519,319 

/*:
100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 9100 9200 9300 9400 9500 9600 9700 9800 9900 10000 10100 10200 10300 10400 10500 10600 10700 10800 10900 11000 11100 11200 11300 11400 11500 11600 11700 11800 11900 12000 12100 12200 12300 12400 12500 12600 12700 12800 12900 13000 13100 13200 13300 13400 13500 13600 13700 13800 13900 14000 14100 14200 14300 14400 14500 14600 14700 14800 14900 15000 15100 15200 15300 15400 15500 15600 15700 15800 15900 16000 
Length(RleTableDataBlob) = 1519319;

//:*/

StringTableIndex = J( Nitems( iFeatureToName )+1, 1, . );

null = Hex To Blob( "00" );

StringTableDataBlob = null; // water. represented by a zero-length null terminated string.

StringTableIndex[1] = 0; // pointer to water

/*

at the very top, a table to convert from name to rule was loaded. make a lookup AA

and then translate the feature name to the rule and put the rule in the blob.

You won't care about the Nuuk/Godthab issue, I hope. It shows up here if Nuuk

is unknown by the gen-tz.py code mentioned earlier as key not found. A kludge

fix is shown, it might need more. My fix involved putting all the items in

iFeatureToName into gen-tz.py; that allowed it to lookup the rule in the linux system.

*/


/*:

0
//:*/

xlate=associativearray(dtTrans:from<<getvalues,dtTrans:to<<getvalues); // rename col1=from, col2=to

// 'America/Nuuk' is 'America/Godthab'

//if(!xlate<<contains("America/Nuuk"),

//xlate["America/Nuuk"] = xlate["America/Godthab"];

//);

for( i = 1, i <= Nitems( iFeatureToName ), i += 1,

	StringTableIndex[i+1] = Length( StringTableDataBlob );

	tznamet=try(xlate[iFeatureToName[i]],"unknown");// "Asia/Qostanay" is new this time

	StringTableDataBlob = StringTableDataBlob || (chartoblob(tznamet) || null);

	//show(i,chartoblob(iFeatureToName[i]),length(StringTableDataBlob));

);

Length( StringTableDataBlob );//7022

/*:

5818
//:*/
loc(iFeatureToName,"America/New_York");
/*:

[152]
//:*/

stiLength = 4*nitems(StringTableIndex);// 4-byte offsets, string table's length

rtiLength = 4*nitems(RleTableIndex); // 4-byte offsets, run table's length

stdLength = length(StringTableDataBlob);

n=12;// number of 4-byte integers, not counting the first n value, counting the "END." value

// first build a matrix of 4-byte integers. some are offsets, some are values.

// 4+n*4 is the header length, including the initial n value.

header = n // number of offsets and ints that follow:

	||((4+n*4)+0) // offset to string index table

	||((4+n*4)+stiLength) // offset to string data "America/New_York\0" and similar packed strings

	||((4+n*4)+stiLength+stdLength) // offset to rle index table

	||((4+n*4)+stiLength+stdLength+rtiLength) // offset to rle data 1 or 2 compressed

	||((4+n*4)+stiLength+stdLength+rtiLength+length(RleTableDataBlob)) // date. sanity checking. not used for much.

	||nrows(composition) // lat

	||ncols(composition) // lon

	||-90||90 // lat coverage for nrows

	||-180||180 // lon cover for ncols

	||hextonumber(hex(reverse("END.")));// little endian reverses again (hextonumber data is big endian)

if(n!=nitems(header)-1,throw("header"));

/*

convert the header matrix to a blob

*/

headerBlob = matrixtoblob(header,"int",4,"little");

hbLength = length(headerBlob);



/*

Convert the string table index to a blob. NOTE the multiply by 0! these are not

pointers and not relocated offsets. they are relative offsets within the string table data

(distinguish between string table INDEX and string table DATA; the first item in the

INDEX is the offset of the first string in the DATA.)

*/

stiBlob = matrixToBlob(StringTableIndex+0*(hbLength+stiLength/*relocate ptrs*/),"int",4,"little");// NOT relocated, index offsets relative to StringTableDataBlob

if(length(stiBlob)!=stiLength,throw("stiBlob"));



/* ditto the RLE table INDEX and DATA */

rtiBlob = matrixtoblob(RleTableIndex+0*(hbLength+stiLength+rtiLength/*relocate ptrs*/),"int",4,"little");// NOT relocated,... RleTableDataBlob

if(length(rtiBlob)!=rtiLength,throw("rtiBlob"));



/* put it together, in order */

finalblob = headerBlob 

			|| stiBlob 

			|| StringTableDataBlob 

			|| rtiBlob 

			|| RleTableDataBlob 

			|| chartoblob("TZ lookup built "||char(asdate(today()))) || null;



write("\!nfinalblob size=",length(finalblob));// pass1=1590571, pass2=1590932

/*:

finalblob size=1590932

//:*/

write(

	"\!nstring NY=",

	blobpeek(finalblob,

		blobtomatrix( 

			Blob Peek( finalblob, 

				Blob To Matrix( Blob Peek( finalblob, 1/*string index*/ * 4, 4 ), "int", 4, "little" )[1]/*offset to index*/ + 4 * 152/*offset in index to offset of NY in string data*/

				, 4 

			),

			"int",4,"little"

		)

		+

		Blob To Matrix( Blob Peek( finalblob, 2/*data*/ * 4, 4 ), "int", 4, "little" )[1]/*offset to start of string data*/

		,

		17

	)

);

/*:

string NY=Char To Blob( "EST5EDT,M3.2.0,M1", "ascii~hex" )

//:*/

write("\!nsignature=",

	blobpeek(

		finalblob,

		Blob To Matrix( Blob Peek( finalblob, 5 * 4, 4 ), "int", 4, "little" )[1],

		300

	)

);

/*:

signature=Char To Blob( "TZ lookup built 20Oct2021:23:32:36~00", "ascii~hex" )

//:*/
savetextfile("f:/ClockScripts/xtzblob.bin", finalblob);
/*:

"f:\ClockScripts\xtzblob.bin"
//:*/

lookup = function({lat=41.91,lon},{row,col,r,c,code},

	row=floor(interpolate(lat,-90,N Rows( composition ),90,1+0))-0;

	r=RleTableIndex[row];

	col=round(interpolate(lon,-180,1+0,180,N Cols( composition )))-0;

	c=0;

	while(c<col,

		code = blobtomatrix(blobpeek(RleTableDataBlob,r,1),"int",1,"big")[1];

		if(code<0,

			code = -blobtomatrix(blobpeek(RleTableDataBlob,r,2),"int",2,"big")[1];

			r+=2;//

		,//

			r+=1;

		);

		length = blobtomatrix(blobpeek(RleTableDataBlob,r,1),"int",1,"big")[1];

		if(length<0,

			length = -blobtomatrix(blobpeek(RleTableDataBlob,r,2),"int",2,"big")[1];

			r+=2;//

		,//

			r+=1;

		);

		c += length;

	);

	if(code>0,

		iFeatureToName[code];

	,//

		"water"

	)

);



/*

draw a bunch of markers, color coded for timezone, over a map.

This give a view of the accuracy near rivers and obvious political boundaries.

*/

//////////////////////////////////////////////////////////////////

markersOver = function({clat,clon,name,grid=1},

	dtVat=New Table( name,

		New Column( "tz", Character, "Nominal" ),

		New Column( "lat", Numeric, "Continuous", Format( "Best", 12 ) ),

		New Column( "lon", Numeric, "Continuous", Format( "Best", 12 ) )

	);



	for(ylat=clat-.5*grid, ylat<=clat+.5*grid,ylat+=.004*grid,

		for(xlon=clon-.5*grid,xlon<=clon+.5*grid,xlon+=.006*grid,

			dtVat<<addrows(1);

			dtVat:tz=lookup(ylat,xlon);

			dtVat:lat=ylat;

			dtVat:lon=xlon;

		)

	);

	dtVat:tz<<label(1);

	dtVat<< Color or Mark by Column(dtVat:tz);

	dtVat<<setdirty(0);

	dtVat = Graph Builder(

		Size( 1843, 976 ),

		Show Control Panel( 0 ),

		Variables( X( :lon ), Y( :lat ) ),

		Elements( Points( X, Y, Legend( 3 ) ) ),

		SendToReport(

//			Dispatch(

//				{},

//				"lon",

//				ScaleBox,

//				{Min( 12.4031559352746 ), Max( 12.5218440647255 ), Inc( 0.01 ),

//				Minor Ticks( 1 )}

//			),

//			Dispatch(

//				{},

//				"lat",

//				ScaleBox,

//				{Scale( "Geodesic" ), Format( "Best", 12 ), Min( 41.8801686871589 ),

//				Max( 41.9397483777795 ), Inc( 0.01 ), Minor Ticks( 4 )}

//			),

			Dispatch(

				{},

				"Graph Builder",

				FrameBox,

				//{Background Map( Images( "Street Map Service" ) ), Marker Size( 6 )}

				{Background Map(

					Images(

						"Web Map Service", "https://ows.terrestris.de/osm-gray/service?",

						"OSM-WMS"

					)

				), Marker Size( 1 ), Marker Drawing Mode( "Normal" )}

			)

		)

	);

);

/*

the smallest timezone *does* align part way under the data. The pixels in the data are bigger than 

this timezone and it falls on the edge of a pixel.

Kansas is interesting because of its western counties.

*/

markersover(41.905,12.445,"vatican city");

/*:

filesnapper watchdog callback

Graph Builder[]
Hover Label: Preset library found at /C:/Program Files/SAS/JMPEA/17/Resources/Builtins/hllib.jsl

// Report snapshot: vatican city - Graph Builder
Data Table( "vatican city" ) << Graph Builder(
    Size( 1825, 905 ),
    Show Control Panel( 0 ),
    Variables( X( :lon ), Y( :lat ) ),
    Elements( Points( X, Y, Legend( 3 ) ) ),
    SendToReport(
        Dispatch(
            {},
            "lon",
            ScaleBox,
            {Format( "Best", 11 ), Min( 12.4430037618781 ), Max( 12.4643578230892 ),
            Inc( 0.005 ), Minor Ticks( 4 )}
        ),
        Dispatch(
            {},
            "lat",
            ScaleBox,
            {Format( "Best", 11 ), Min( 41.898775812282 ), Max( 41.9086888668756 ),
            Inc( 0.002 ), Minor Ticks( 3 )}
        ),
        Dispatch(
            {},
            "Graph Builder",
            FrameBox,
            {Background Map(
                Images(
                    "Web Map Service", "https://ows.terrestris.de/osm-gray/service?",
                    "OSM-WMS"
                )
            ), Marker Size( 6 ), Marker Drawing Mode( "Normal" )}
        )
    )
);


// Close Data Table: vatican city
Close( "vatican city", NoSave );

filesnapper watchdog callback

//:*/
markersover(38, -85, "Kentucky",20) ;
/*:

filesnapper watchdog callback

Graph Builder[]

// Report snapshot: Kentucky - Graph Builder
Data Table( "Kentucky" ) << Graph Builder(
    Size( 1825, 905 ),
    Show Control Panel( 0 ),
    Variables( X( :lon ), Y( :lat ) ),
    Elements( Points( X, Y, Legend( 3 ) ) ),
    SendToReport(
        Dispatch(
            {},
            "lon",
            ScaleBox,
            {Format( "Best", 9 ), Min( -92.3243682514557 ), Max( -78.3924237284942 ),
            Inc( 2 ), Minor Ticks( 1 )}
        ),
        Dispatch(
            {},
            "lat",
            ScaleBox,
            {Format( "Best", 12 ), Min( 35.2691911482968 ), Max( 41.5824743563169 ),
            Inc( 1 ), Minor Ticks( 1 )}
        ),
        Dispatch(
            {},
            "Graph Builder",
            FrameBox,
            {Background Map(
                Images(
                    "Web Map Service",
                    "https://ows.terrestris.de/osm-gray/service?",
                    "OSM-WMS",
                    Transparency( 0 )
                ),
                Boundaries( {"US States"} )
            ), Marker Size( 6 ), Marker Drawing Mode( "Normal" ),
            Grid Line Order( 1 ), Reference Line Order( 2 ),
            Reorder Segs( {1, 2, 5, 4} ), DispatchSeg(
                Shape Seg( 1 ),
                {Color( "Black" ), Line Width( 9 )}
            )}
        )
    )
);


// Close Data Table: Kentucky
Close( "Kentucky", NoSave );

filesnapper watchdog callback

//:*/
markersover(35, -78,"home",100);
/*:

filesnapper watchdog callback

Graph Builder[]

// Report snapshot: home - Graph Builder
Data Table( "home" ) << Graph Builder(
    Size( 1825, 905 ),
    Show Control Panel( 0 ),
    Variables( X( :lon ), Y( :lat ) ),
    Elements( Points( X, Y, Legend( 3 ) ) ),
    SendToReport(
        Dispatch(
            {},
            "lon",
            ScaleBox,
            {Format( "Best", 9 ), Min( -76.207831037278 ), Max( -65.0835179610094 ),
            Inc( 2 ), Minor Ticks( 1 )}
        ),
        Dispatch(
            {},
            "lat",
            ScaleBox,
            {Format( "Best", 9 ), Min( 16.1286853216062 ), Max( 21.1696857956905 ),
            Inc( 1 ), Minor Ticks( 4 )}
        ),
        Dispatch(
            {},
            "Graph Builder",
            FrameBox,
            {Background Map(
                Images(
                    "Web Map Service", "https://ows.terrestris.de/osm-gray/service?",
                    "OSM-WMS"
                ),
                Boundaries( {"US States"} )
            ), Marker Size( 6 ), Marker Drawing Mode( "Normal" ),
            DispatchSeg( Shape Seg( 1 ), {Color( "Black" ), Line Width( 3 )} )}
        )
    )
);


// Close Data Table: home
Close( "home", NoSave );

filesnapper watchdog callback


// Close Data Table: zones
Close( "zones", NoSave );

filesnapper watchdog callback

