# A demo HTTP file upload server modified from:
#   http://flask.pocoo.org/docs/1.0/patterns/fileuploads/#uploading-files
# This server is used to test examples/form_client.
# Usage:
# 1. Copy this file to some other place.
# 2. Change UPLOAD_FOLDER to an existing local folder.
# (Run on Windows)
# 3. $ set FLASK_APP=form_server_flask.py
# 4. $ flask run

import os
from flask import Flask, flash, request, redirect, url_for
from flask import send_from_directory
from werkzeug.utils import secure_filename


# NOTE: Please change this to an existing folder.
UPLOAD_FOLDER = '/path/to/the/uploads'

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


@app.route('/upload', methods=['POST'])
def upload_file():
    if request.method == 'POST':
        for name, data in request.form.items():
            print(f'form: {name}, {data}')

        for name, file in request.files.items():
            if file:
                secured_filename = secure_filename(file.filename)
                print(f"file:  {name}, {file.filename} ({secured_filename})")
                file.save(os.path.join(app.config['UPLOAD_FOLDER'],
                    secured_filename))

        return "OK"


@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(app.config['UPLOAD_FOLDER'], filename)
