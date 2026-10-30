fs = import('fs')

json_files = [
    'standard_compliant.json',
    'test_edge_cases.json',
    'test_invalid.json',
    'test.json',
]

foreach f : json_files
    fs.copyfile(f, f)
endforeach