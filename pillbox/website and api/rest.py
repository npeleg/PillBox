# RestAPI function and classed
from flask_restful import Resource, Api, reqparse
from utils import *

parser = reqparse.RequestParser()
parser.add_argument('board_id')

class publishTopicFromBoard(Resource):
	def post(self):
		args = parser.parse_args()
		topic = args['board_id']
		current_topics = getAllTopics()
		if topic not in current_topics:
			print("ERROR: publish request from non-existing client: " + topic)
			return "ERROR", 404
		print("Publishing from board to topic:" + str(topic))
		publishTopic(topic, "Pillbox alert: Pills were not taken.")
		return "Seccessfully published to topic: " + str(topic), 201
