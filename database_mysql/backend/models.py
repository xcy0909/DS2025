from flask_sqlalchemy import SQLAlchemy
from config import SQLALCHEMY_DATABASE_URI

db = SQLAlchemy()

# 用户模型
class User(db.Model):
    __tablename__ = 'users'
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(50), unique=True, nullable=False)
    password = db.Column(db.String(100), nullable=False)  # 存储加密后的密码
    role = db.Column(db.String(20), default='teacher')  # role: teacher/admin

# 学生模型
class Student(db.Model):
    __tablename__ = 'students'
    student_id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(50), nullable=False)
    gender = db.Column(db.String(10), nullable=False)
    grade = db.Column(db.String(20), nullable=False)  # 年级（如2023级）
    major = db.Column(db.String(50), nullable=False)  # 专业（如计算机科学）
    create_time = db.Column(db.DateTime, default=db.func.current_timestamp())
    # 关联成绩表（一对多）
    scores = db.relationship('Score', backref='student', lazy=True, cascade='all, delete-orphan')

# 成绩模型
class Score(db.Model):
    __tablename__ = 'scores'
    score_id = db.Column(db.Integer, primary_key=True)
    student_id = db.Column(db.Integer, db.ForeignKey('students.student_id'), nullable=False)
    course = db.Column(db.String(50), nullable=False)
    score = db.Column(db.Integer, nullable=False)
    exam_time = db.Column(db.Date, nullable=False)