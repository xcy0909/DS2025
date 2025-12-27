from flask import Flask, request, jsonify, render_template, redirect, url_for, session
from flask_cors import CORS
from flask_session import Session  # 会话管理
from models import db, User, Student, Score
from config import SQLALCHEMY_DATABASE_URI, SECRET_KEY
import datetime
from sqlalchemy import text
from werkzeug.security import check_password_hash, generate_password_hash  # 密码工具

app = Flask(__name__, static_folder='../frontend', template_folder='../frontend')
# 配置加载
app.config['SQLALCHEMY_DATABASE_URI'] = SQLALCHEMY_DATABASE_URI
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SECRET_KEY'] = SECRET_KEY
# 会话配置加载
app.config['SESSION_TYPE'] = 'filesystem'
app.config['SESSION_FILE_DIR'] = './flask_sessions'
app.config['SESSION_PERMANENT'] = False
app.config['PERMANENT_SESSION_LIFETIME'] = 3600
app.config['SESSION_USE_SIGNER'] = True

CORS(app)  # 解决跨域问题
db.init_app(app)
Session(app)  # 初始化会话功能


# ---------------------- 登录/登出接口 ----------------------
# 登录页面路由
@app.route('/login')
def login_page():
    # 已登录则直接跳转到主页
    if 'username' in session:
        return redirect(url_for('index'))
    return render_template('login.html')


# 登录验证接口（POST）
@app.route('/api/login', methods=['POST'])
def login():
    data = request.get_json()
    username = data.get('username')
    password = data.get('password')

    # 校验参数
    if not username or not password:
        return jsonify({"code": 400, "msg": "用户名和密码不能为空"})

    # 查询用户
    user = db.session.query(User).filter_by(username=username).first()
    if not user:
        return jsonify({"code": 401, "msg": "用户名不存在"})

    # 密码验证（如果是加密存储，使用check_password_hash(user.password, password)）
    # 明文验证（建议后续改为加密）
    if user.password != password:
        return jsonify({"code": 401, "msg": "密码错误"})

    # 登录成功：存储会话信息
    session['username'] = user.username
    session['role'] = user.role
    session['user_id'] = user.id

    return jsonify({"code": 200, "msg": "登录成功", "data": {"role": user.role}})


# 登出接口
@app.route('/api/logout', methods=['POST'])
def logout():
    # 清除会话信息
    session.clear()
    return jsonify({"code": 200, "msg": "登出成功"})


# 登录验证装饰器
def login_required(f):
    from functools import wraps
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'username' not in session:
            # 未登录：返回403
            return jsonify({"code": 403, "msg": "请先登录"}), 403
        return f(*args, **kwargs)

    return decorated_function


# ---------------------- 学生管理CRUD接口 ----------------------
# 1.1 创建学生（C）
@app.route('/api/students', methods=['POST'])
@login_required
def add_student():
    data = request.get_json()
    try:
        new_student = Student(
            name=data['name'],
            gender=data['gender'],
            grade=data['grade'],
            major=data['major']
        )
        db.session.add(new_student)
        db.session.commit()
        return jsonify({"code": 200, "msg": "学生添加成功", "data": new_student.student_id})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"添加失败：{str(e)}"})


# 1.2 查询所有学生（R）- 新增按条件筛选
@app.route('/api/students', methods=['GET'])
@login_required
def get_all_students():
    # 可选筛选参数
    grade = request.args.get('grade')
    major = request.args.get('major')

    query = Student.query
    if grade:
        query = query.filter_by(grade=grade)
    if major:
        query = query.filter_by(major=major)

    students = query.all()
    result = []
    for s in students:
        result.append({
            "student_id": s.student_id,
            "name": s.name,
            "gender": s.gender,
            "grade": s.grade,
            "major": s.major,
            "create_time": s.create_time.strftime("%Y-%m-%d %H:%M:%S"),
            "score_count": len(s.scores)  # 关联查询：该学生的成绩记录数
        })
    return jsonify({"code": 200, "data": result})


# 1.3 更新学生信息（U）
@app.route('/api/students/<int:student_id>', methods=['PUT'])
@login_required
def update_student(student_id):
    data = request.get_json()
    student = db.session.get(Student, student_id)
    if not student:
        return jsonify({"code": 404, "msg": "学生不存在"})
    try:
        student.name = data.get('name', student.name)
        student.gender = data.get('gender', student.gender)
        student.grade = data.get('grade', student.grade)
        student.major = data.get('major', student.major)
        db.session.commit()
        return jsonify({"code": 200, "msg": "更新成功"})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"更新失败：{str(e)}"})


# 1.4 删除学生（D）
@app.route('/api/students/<int:student_id>', methods=['DELETE'])
@login_required
def delete_student(student_id):
    student = db.session.get(Student, student_id)
    if not student:
        return jsonify({"code": 404, "msg": "学生不存在"})
    try:
        db.session.delete(student)
        db.session.commit()
        return jsonify({"code": 200, "msg": "删除成功"})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"删除失败：{str(e)}"})


# ---------------------- 成绩表CRUD接口（完善数据操作完整性）----------------------
# 2.1 添加学生成绩（C）
@app.route('/api/scores', methods=['POST'])
@login_required
def add_score():
    data = request.get_json()
    # 参数校验
    required_fields = ['student_id', 'course', 'score', 'exam_time']
    for field in required_fields:
        if field not in data:
            return jsonify({"code": 400, "msg": f"缺少必填参数：{field}"})

    # 验证学生是否存在
    student = db.session.get(Student, data['student_id'])
    if not student:
        return jsonify({"code": 404, "msg": "关联的学生不存在"})

    # 验证成绩范围
    if not (0 <= data['score'] <= 100):
        return jsonify({"code": 400, "msg": "成绩必须在0-100之间"})

    try:
        # 验证日期格式（YYYY-MM-DD）
        datetime.datetime.strptime(data['exam_time'], '%Y-%m-%d')
        new_score = Score(
            student_id=data['student_id'],
            course=data['course'],
            score=data['score'],
            exam_time=data['exam_time']
        )
        db.session.add(new_score)
        db.session.commit()
        return jsonify({"code": 200, "msg": "成绩添加成功", "data": new_score.score_id})
    except ValueError:
        return jsonify({"code": 400, "msg": "日期格式错误，需为YYYY-MM-DD"})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"添加失败：{str(e)}"})


# 2.2 查询所有成绩（R）- 支持按学生ID筛选
@app.route('/api/scores', methods=['GET'])
@login_required
def get_all_scores():
    student_id = request.args.get('student_id')  # 可选参数：按学生ID筛选
    if student_id:
        scores = Score.query.filter_by(student_id=student_id).all()
    else:
        scores = Score.query.all()

    result = []
    for s in scores:
        # 关联查询学生姓名（多表关联查询）
        student = db.session.get(Student, s.student_id)
        result.append({
            "score_id": s.score_id,
            "student_id": s.student_id,
            "student_name": student.name if student else "未知学生",
            "course": s.course,
            "score": s.score,
            "exam_time": s.exam_time.strftime("%Y-%m-%d")
        })
    return jsonify({"code": 200, "data": result})


# 2.3 更新成绩信息（U）
@app.route('/api/scores/<int:score_id>', methods=['PUT'])
@login_required
def update_score(score_id):
    data = request.get_json()
    score = db.session.get(Score, score_id)
    if not score:
        return jsonify({"code": 404, "msg": "成绩记录不存在"})

    try:
        # 可选更新字段：课程、成绩、考试时间
        if 'course' in data:
            score.course = data['course']
        if 'score' in data:
            if 0 <= data['score'] <= 100:
                score.score = data['score']
            else:
                return jsonify({"code": 400, "msg": "成绩必须在0-100之间"})
        if 'exam_time' in data:
            datetime.datetime.strptime(data['exam_time'], '%Y-%m-%d')
            score.exam_time = data['exam_time']

        db.session.commit()
        return jsonify({"code": 200, "msg": "成绩更新成功"})
    except ValueError:
        return jsonify({"code": 400, "msg": "日期格式错误，需为YYYY-MM-DD"})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"更新失败：{str(e)}"})


# 2.4 删除成绩（D）
@app.route('/api/scores/<int:score_id>', methods=['DELETE'])
@login_required
def delete_score(score_id):
    score = db.session.get(Score, score_id)
    if not score:
        return jsonify({"code": 404, "msg": "成绩记录不存在"})
    try:
        db.session.delete(score)
        db.session.commit()
        return jsonify({"code": 200, "msg": "成绩删除成功"})
    except Exception as e:
        db.session.rollback()
        return jsonify({"code": 500, "msg": f"删除失败：{str(e)}"})


# ---------------------- 复杂查询接口（强化稳定性）----------------------
# 原有复杂查询：各专业课程成绩统计（多表联接+聚合函数）
@app.route('/api/complex/score-stat', methods=['GET'])
@login_required
def score_statistics():
    try:
        query = text("""
        SELECT s.major, c.course, 
               ROUND(AVG(c.score), 2) as avg_score, 
               MAX(c.score) as max_score, 
               MIN(c.score) as min_score,
               COUNT(c.score) as student_count  # 新增：统计参与人数
        FROM students s
        INNER JOIN scores c ON s.student_id = c.student_id
        GROUP BY s.major, c.course
        ORDER BY s.major, avg_score DESC
        """)
        result = db.session.execute(query).mappings().all()
        stat_data = [dict(item) for item in result]  # 简化格式化
        return jsonify({"code": 200, "data": stat_data})
    except Exception as e:
        return jsonify({"code": 500, "msg": f"查询失败：{str(e)}"}), 500


# 新增复杂查询：按年级统计各课程及格率（多表联接+聚合函数+条件判断）
@app.route('/api/complex/pass-rate', methods=['GET'])
@login_required
def course_pass_rate():
    try:
        query = text("""
        SELECT s.grade, c.course,
               COUNT(c.score) as total_students,
               SUM(CASE WHEN c.score >= 60 THEN 1 ELSE 0 END) as pass_students,
               ROUND((SUM(CASE WHEN c.score >= 60 THEN 1 ELSE 0 END) / COUNT(c.score)) * 100, 2) as pass_rate
        FROM students s
        INNER JOIN scores c ON s.student_id = c.student_id
        GROUP BY s.grade, c.course
        ORDER BY s.grade, pass_rate DESC
        """)
        result = db.session.execute(query).mappings().all()
        pass_rate_data = [dict(item) for item in result]
        return jsonify({"code": 200, "data": pass_rate_data})
    except Exception as e:
        return jsonify({"code": 500, "msg": f"查询失败：{str(e)}"}), 500


# ---------------------- 前端页面路由 ----------------------
@app.route('/')
def index():
    # 未登录则跳转到登录页
    if 'username' not in session:
        return redirect(url_for('login_page'))
    return render_template('index.html')


if __name__ == '__main__':
    # 启动时创建数据库表（如果不存在）
    with app.app_context():
        db.create_all()
    app.run(debug=True, port=5000)