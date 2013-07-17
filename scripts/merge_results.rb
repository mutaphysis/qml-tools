#! /usr/bin/ruby

require "rubygems"
require "json"

file_name = ARGV[0] ||  "coverage_data.json"

json_data = JSON.parse(IO.read(file_name))

merged_data = {}

def merge(source, target, key)
	return if source[key].nil?

	if target[key].nil?
		max_index = 0
		target[key] = Hash.new
	else
		highest = target[key].max_by{ |k,v| k.to_i }
		max_index =  highest.nil? ? 0 : highest[0].to_i
	end

	source[key].each { |k, v|
		new_index = max_index + k.to_i
		target[key][new_index.to_s] = v
	}
end

# merge results
json_data.each { |key, val|

	names = key.split(':')
	if (names.length == 3)
		key = names[0]
		current = merged_data[key]

		if current.nil?
			val['path'] = key
			merged_data[key] = val
		else
			merge(val, current, 's')
			merge(val, current, 'b')
			merge(val, current, 'f')
			merge(val, current, 'fnMap')
			merge(val, current, 'statementMap')
			merge(val, current, 'branchMap')
		end
	else
		merged_data[key] = val
	end
}

print JSON.pretty_generate(merged_data)