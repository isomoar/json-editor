SchemaBasedJSONEditor (v. 0.1)
=====================

Schema-based JSON editor

With this text editor you can simply create JSON documents and check it according to schema files.

Feautures:
- autocompletion for key words
- autoindents
- syntax highlighting and analysing
- document validation (used tv4 validator for v4 JSON Schema) https://github.com/geraintluff/tv4

Schema-file is linked to each JSON-document by adding in the document its path:
"$schema" : "/path_to_schema_file.json"

Type CTR+E (or cmd+E in Mac OS) to call completer.


