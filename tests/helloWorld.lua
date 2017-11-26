print("Hello World!")

if (argc > 0)
then
	print("Passed "..argc.." parameter(s):")
	for i,v in ipairs(argv)
	do
		print(" "..i..": "..v)
	end
end

-- Calling dummyLib's 'helloWorld' method
if(dummyLib ~= nil)
then
	dummyLib.helloWorld()
else
	print("dummyLib not loaded yet")
end

local lrbi = lrbi;
lrbi.sleep(200)
lrbi.require("Dummy")

-- Calling dummyLib's 'helloWorld' method
if(dummyLib ~= nil)
then
	local d = dummyLib;
	d.helloWorld()

	local t = d.getTable()
	-- Using table enumerator
	for k,v in pairs(t)
	do
		print("Key="..k.." Value="..tostring(v).." ("..type(v)..")")
	end
	-- Using dot syntax
	print("Name="..t.name)
	-- Using bracket syntax
	print("Num="..tostring(t['num']))

	t[1] = 23
	t.addValue = 550
	d.printTable(t)

	d.varParams(1.2, "test", nil, -5, t, false)

	d.optParams("str1", nil)
	d.optParams("str2")
	d.optParams("str3", 6)

else
	error("dummyLib not loaded!")
end

return 1 -- Optional return value
