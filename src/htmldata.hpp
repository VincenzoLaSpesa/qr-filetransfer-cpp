#pragma once
const char *html = R"MULTILINE(
<!doctype html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, user-scalable=no">
    <title>qr-filetransfer-cpp</title>
</head>

<body>
	<form action="/upload/{0}" method="post" enctype="multipart/form-data">
	<label for="files">
		Files to transfer
	</label>
	<input class="form-control-file" type="file" name="files" id="files" multiple>
	<br/>
	<input class="btn btn-primary form-control form-control-lg" type="submit" name="submit" value="Transfer">
	</form>
</body>
</html>
)MULTILINE";