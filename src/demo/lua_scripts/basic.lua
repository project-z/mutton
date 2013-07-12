-- Copyright (c) 2013 Matthew Stump

-- This file is part of libmutton.

-- libmutton is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as
-- published by the Free Software Foundation, either version 3 of the
-- License, or (at your option) any later version.

-- libmutton is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.

-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

print("\nTROLOLOL I'm a script!!");

------------------------------------------------------------------------------------
--
-- Debug
--
------------------------------------------------------------------------------------
function print_table(table)
   for key, value in pairs(table) do print("\t", string.format("%-25s", key), "=>", value) end
end

print("\nMutton Values:")
print_table(mutton)

print("\nGlobal Values:")
print_table(_G)

-- print("\Package Values:")
-- print_table(package)

------------------------------------------------------------------------------------
--
-- Let's parse an event
--
------------------------------------------------------------------------------------

print("\nLet's parse an event")
print("Got following JSON data as event body:")
print(mutton.event_data)

local json = require("dkjson")
local event, pos, err = json.decode(mutton.event_data, 1, nil)
if err then
   print("\nError parsing event JSON:", err)
else
   print("\nSuccessfully parsed the event JSON!")
   print("The value for the event field 'a_field':")
   print(event.a_field)

   print("\nIndexing the data for user '1'")
   mutton_index_value_trigram(mutton.context, 1, mutton.bucket, "a_field", event.a_field, 1, true)
end


------------------------------------------------------------------------------------
--
-- Unit tests
--
------------------------------------------------------------------------------------

if pcall(
   function()
      mutton_index_value(mutton.context, 1, 1, 1, "1234567812345678  ", "1234567812345678", true)
   end)
then
   error("should reject because value is too long")
end

if pcall(
   function()
      mutton_index_value_trigram(mutton.context, "a", 1, 1, "1234567812345678  ", "1234567812345678", true)
   end)
then
   error("should reject because partition it isn't an int")
end

if pcall(
   function()
      mutton_index_value_trigram(mutton.context, 1, 1, 1, "1234567812345678", "1234567812345678  ", true)
   end)
then
   error("should reject because who_or_what is too long")
end

if pcall(
   function()
      mutton_index_value_trigram(1, 1, 1, 1, "1234567812345678  ", "1234567812345678", true)
   end)
then
   error("should reject because context isn't light userdata")
end

if pcall(
   function()
      mutton_index_value_trigram(mutton.context, 1, mutton.bucket, "a_field", nil, 1, true)
   end)
then
   error("should reject because value is nil")
end



print("\nTHE END");