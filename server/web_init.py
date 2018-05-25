import sqlite3
from flask import Flask, g, render_template
from contextlib import closing

# Configuration of the sql database
DATABASE = '/Users/angus/PycharmProjects/plantr/plant.db'
DEBUG = True

# Create an instance of the application
app = Flask(__name__)
app.config.from_object(__name__)


def connect_db():
    return sqlite3.connect(app.config['DATABASE'])


def init_db():
    with closing(connect_db()) as db:
        with app.open_resource('schema.sql', mode='r') as f:
            db.cursor().executescript(f.read())
        db.commit()


@app.before_request
def before_request():
    print "connect"
    g.db = connect_db()


@app.teardown_request
def teardown_request(exception):
    print "dc"
    db = getattr(g, 'db', None)
    if db is not None:
        db.close()


@app.route('/')
def web_index():
    return render_template('index.html')


@app.route('/metrics')
def web_metrics_overview():
    return render_template('metrics/overview.html')


@app.route('/metrics/temp')
def web_metrics_temp():
    cur = g.db.execute('select time, data, temperature from entries order by id desc limit 10')
    entries_metric_temp = [dict(time=row[0], temperature=row[1]) for row in cur.fetchall()]
    return render_template('metrics/temp.html', entries=entries_metric_temp)


@app.route('/metrics/ph')
def web_metrics_ph():
    return render_template('metrics/ph.html')


@app.route('/metrics/humidity')
def web_metrics_humidity():
    return render_template('graph/humidity.html')


@app.route('/graph/temp')
def web_graph_temp():
    return render_template('graph/temp.html')


@app.route('/graph/ph')
def web_graph_ph():
    return render_template('graph/ph.html')


@app.route('/graph/humidity')
def web_graph_humidity():
    return render_template('graph/humidity.html')


@app.route('/database')
def web_database():
    entries_database = []
    try:
        cur = g.db.execute('select time, data from entries order by id desc')
        entries_database = [dict(time=row[0], data=row[1]) for row in cur.fetchall()]
    except sqlite3.OperationalError:
        print "locked"
    return render_template('database.html', entries=entries_database)


@app.route('/manage')
def web_manage():
    return render_template('manage.html')


# Finally run the server.
if __name__ == '__main__':
    app.run()
