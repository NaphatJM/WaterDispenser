from flask import Flask, jsonify, render_template, request, json

app = Flask(__name__)

data_file = 'dispensed_data.json'  # Same file used in Test.py

# Route to retrieve dispensed data
@app.route('/api/data', methods=['GET'])
def get_data():
    try:
        with open(data_file, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        data = []
    return jsonify(data)

@app.route('/api/data', methods=['POST'])
def post_data():
    new_data = request.get_json()  # Expecting JSON data from the client

    # Load existing data from file
    try:
        with open(data_file, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        data = []

    # Add the new dispensed data to the existing data
    data.append(new_data)

    # Save the updated data back to the file
    with open(data_file, 'w') as f:
        json.dump(data, f, indent=4)

    return jsonify({"message": "Data received successfully"}), 200


# Dashboard route
@app.route('/')
def dashboard():
    try:
        with open(data_file, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        data = []
    return render_template('dashboard.html', dispensed_data=data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
